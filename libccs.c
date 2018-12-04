/* libccs.c - Implements interface with the parser and the Prolog Engine
Copyright (C) 2009  Simonetto, Perfetti, Qorri

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include "gprolog.h"
#include "libccs.h"
#include "ccs_parse.h"

#define ccs_print_plterm(t) ccs_fprint_plterm(stdout, t)

extern int ccs__scan_string(const char *str);
extern int ccs_parse(proc_t *p);
extern void ccs_parse_process();
extern void ccs_parse_transition();
extern void ccs_parse_definition();

static void ccs_fprint_plterm(FILE *fp, PlTerm term);
static void ccs_build_lts(proc_t p, proc_t states_list, int *state_counter, proc_t *act_list);
static proc_t lts_state_insert(proc_t p, int *state_counter);
static void lts_trans_insert(proc_t orig, proc_t dest, proc_t act);
static void lts_clean();

static void ccs_build_lts(proc_t p, proc_t parent_label, int *state_counter, proc_t *act_list)
{
PlTerm arg[3];
PlTerm act[2];
PlTerm state[2];
PlTerm state_label;
Bool res;

	arg[0] = p;
	arg[1] = Mk_Variable();
	arg[2] = Mk_Variable();

	Pl_Query_Begin(TRUE);
	res = Pl_Query_Call(Pl_Find_Atom("move"), 3, arg);
	while(res) {
		act[1] = *act_list;
		act[0] = arg[2];
		*act_list = Mk_List(act);
		state[0] = Mk_Variable();
		state[1] = arg[1];
		
		/* If this state isn't stored yet, do it now */
		Pl_Query_Begin(TRUE);
		if(!Pl_Query_Call(Pl_Find_Atom("lts_state"), 2, state)) {
			state_label = lts_state_insert(arg[1], state_counter);
			lts_trans_insert(parent_label, state_label, arg[2]);
			ccs_build_lts(arg[1], state_label, state_counter, act_list);
		} else
			lts_trans_insert(parent_label, state[0], arg[2]);
		Pl_Query_End(PL_RECOVER);
		res = Pl_Query_Next_Solution();
	}
	Pl_Query_End(PL_RECOVER);
}

static void lts_trans_insert(proc_t orig, proc_t dest, proc_t act)
{
PlTerm trans[4];

	trans[0] = orig;
	trans[1] = dest;
	trans[2] = act;

	Pl_Query_Begin(TRUE);
	trans[3] = Mk_Compound(Pl_Create_Allocate_Atom("lts_trans"), 3, trans);
	Pl_Query_Call(Pl_Find_Atom("assertz"), 1, trans + 3);
	Pl_Query_End(PL_RECOVER);
}

static proc_t lts_state_insert(proc_t p, int *state_counter)
{
PlTerm state[3];
char str_counter[11];

	sprintf(str_counter, "%d", *state_counter);
	state[0] = Mk_String(str_counter);
	state[1] = p;

	Pl_Query_Begin(TRUE);
	(*state_counter)++;
	state[2] = Mk_Compound(Pl_Create_Allocate_Atom("lts_state"), 2, state);
	Pl_Query_Call(Pl_Find_Atom("assertz"), 1, state + 2);
	Pl_Query_End(PL_RECOVER);
	return state[0];
}

void ccs_print_lts(proc_t p1)
{
PlTerm arg[3];
Bool res;
PlTerm state_label;
PlTerm act_list;
int state_counter = 0;

	act_list = Mk_List(0);
	state_label = lts_state_insert(p1, &state_counter);
	ccs_build_lts(p1, state_label, &state_counter, &act_list);

	arg[0] = Mk_Variable();
	arg[1] = Mk_Variable();

	Pl_Query_Begin(TRUE);
	res = Pl_Query_Call(Pl_Find_Atom("lts_state"), 2, arg);
	if(res) {
		printf("\n\tInitial state q0 = ");
		ccs_print_process(arg[1]);
		printf("\n\n\tSet of the states Q = {\n");
		while(res) {
			printf("\t\t[");
			ccs_print_plterm(arg[0]);
			printf("] = ");
			ccs_print_process(arg[1]);
			printf("\n");
			res = Pl_Query_Next_Solution();
		}
		printf("\t}\n");
	}
	Pl_Query_End(PL_RECOVER);

	Pl_Query_Begin(TRUE);
	arg[0] = Mk_Variable();
	arg[1] = Mk_Variable();
	arg[2] = Mk_Variable();
	res = Pl_Query_Call(Pl_Find_Atom("lts_trans"), 3, arg);
	if(res) {
		printf("\n\tTransitions list = {\n");
		while(res) {
			printf("\t\t([");
			ccs_print_plterm(arg[0]);
			printf("], [");
			ccs_print_plterm(arg[1]);
			printf("], ");
			ccs_print_process(arg[2]);
			printf(")\n");
			res = Pl_Query_Next_Solution();
		}
		printf("\t}\n\n");
	}
	Pl_Query_End(PL_RECOVER);
	lts_clean();
}

static void lts_clean()
{
Bool res; // TODO: check me!
PlTerm arg[4];

	Pl_Query_Begin(TRUE);
	arg[0] = Mk_Variable();
	arg[1] = Mk_Variable();
	arg[2] = Mk_Compound(Pl_Find_Atom("lts_state"), 2, arg);
	res = Pl_Query_Call(Pl_Find_Atom("retractall"), 1, arg + 2);
	Pl_Query_End(PL_RECOVER);

	Pl_Query_Begin(TRUE);
	arg[0] = Mk_Variable();
	arg[1] = Mk_Variable();
	arg[2] = Mk_Variable();
	arg[3] = Mk_Compound(Pl_Find_Atom("lts_trans"), 3, arg);
	res = Pl_Query_Call(Pl_Find_Atom("retractall"), 1, arg + 3);
	Pl_Query_End(PL_RECOVER);
}

void ccs_print_lts_svg(proc_t p, const char *outname)
{
PlTerm arg[3];
FILE *dot;
char *cmd = (char*) malloc(40 + strlen(outname));
Bool res;
PlTerm state_label;
PlTerm act_list;
int state_counter = 0;

	/* Use the program `dot' from the package `graphviz' for building graphics */
	sprintf(cmd, "dot -Tsvg -o %s", outname);
	dot = popen(cmd, "w");
	free(cmd);
	if(!dot) {
		perror("muccs");
		return;
	}
	fprintf(dot, "strict digraph lts {");

	act_list = Mk_List(0);
	state_label = lts_state_insert(p, &state_counter);
	ccs_build_lts(p, state_label, &state_counter, &act_list);

	arg[0] = Mk_Variable();
	arg[1] = Mk_Variable();

	Pl_Query_Begin(TRUE);
	res = Pl_Query_Call(Pl_Find_Atom("lts_state"), 2, arg);
	if(res) {
		fprintf(dot, "\n\tn");
		ccs_fprint_plterm(dot, arg[0]);
		fprintf(dot, " [label=\"");
		ccs_fprint_process(dot, arg[1]);
		fprintf(dot, "\", style=bold];");
		res = Pl_Query_Next_Solution();
	}
	while(res) {
		fprintf(dot, "\n\tn");
		ccs_fprint_plterm(dot, arg[0]);
		fprintf(dot, " [label=\"");
		ccs_fprint_process(dot, arg[1]);
		fprintf(dot, "\"];");
		res = Pl_Query_Next_Solution();
	}
	Pl_Query_End(PL_RECOVER);

	arg[0] = Mk_Variable();
	arg[1] = Mk_Variable();
	arg[2] = Mk_Variable();

	Pl_Query_Begin(TRUE);
	res = Pl_Query_Call(Pl_Find_Atom("lts_trans"), 3, arg);
	while(res) {
		fprintf(dot, "\n\tn");
		ccs_fprint_plterm(dot, arg[0]);
		fprintf(dot, "->n");
		ccs_fprint_plterm(dot, arg[1]);
		fprintf(dot, " [label=\"");
		ccs_fprint_process(dot, arg[2]);
		fprintf(dot, "\"];");
		res = Pl_Query_Next_Solution();
	}
	Pl_Query_End(PL_RECOVER);
	lts_clean();
	
	fprintf(dot, "\n}\n");
	if(!pclose(dot)) 
		printf("`%s' successfully created.\n", outname);
	else
		perror("muccs");
}

int ccs_read(const char *buf, proc_t *p, int ccs_type)
{
int val;

	switch (ccs_type) {
		case CCS_PROCESS:
			ccs_parse_process();
			break;
		case CCS_TRANSITION:
			ccs_parse_transition();
			break;
		case CCS_DEFINITION:
			ccs_parse_definition();
			break;
		default:
			fprintf(stderr, "Type of CCS term not defined!\n");
			break;
	}

	ccs__scan_string(buf);
	val = ccs_parse(p);
	return val;
}

void ccs_print_all_defs()
{
PlTerm arg[2];
Bool res;

	arg[0] = Mk_Variable();
	arg[1] = Mk_Variable();

	printf("\n");
	Pl_Query_Begin(TRUE);
	if(!(res = Pl_Query_Call(Pl_Find_Atom("proc_def"), 2, arg))) 
		printf("\t<EMPTY>\n");
	
	while(res) {
		printf("\t");
		ccs_print_process(arg[0]);
		printf(" = ");
		ccs_print_process(arg[1]);
		printf("\n");
		res = Pl_Query_Next_Solution();
	}
	Pl_Query_End(PL_RECOVER);
	printf("\n");
}

void ccs_print_all_moves(proc_t p1)
{
PlTerm arg[3];
Bool res;

	arg[0] = p1;
	arg[1] = Mk_Variable();
	arg[2] = Mk_Variable();

	printf("\n");
	Pl_Query_Begin(TRUE);
	if(!(res = Pl_Query_Call(Pl_Find_Atom("move"), 3, arg)))
		printf("\t<EMPTY>\n");
		
	while(res) {
		printf("\t");
		ccs_print_process(arg[2]);
		printf(" -> ");
		ccs_print_process(arg[1]);
		printf("\n");
		res = Pl_Query_Next_Solution();
	}
	Pl_Query_End(PL_RECOVER);
}

int ccs_can_move(proc_t p1, proc_t p2)
{
PlTerm arg[3];
Bool res;

	arg[0] = p1;
	arg[1] = p2;
	arg[2] = Mk_Variable();

	Pl_Query_Begin(TRUE);
	res = Pl_Query_Call(Pl_Find_Atom("move"), 3, arg);
	Pl_Query_End(PL_RECOVER);
	return res;
}

int ccs_can_reach(proc_t p1, proc_t p2)
{
PlTerm args[3];
Bool res;

	args[0] = p1;
	args[1] = p2;
	args[2] = Mk_Variable();

	Pl_Query_Begin(TRUE);
	res = Pl_Query_Call(Pl_Find_Atom("move_star"), 3, args);
	Pl_Query_End(PL_RECOVER);
	return res;
}

int ccs_test_trans(proc_t p1, proc_t p2, proc_t a1)
{
PlTerm args[3];
Bool res;

	args[0] = p1;
	args[1] = p2;
	args[2] = a1;

	Pl_Query_Begin(TRUE);
	res = Pl_Query_Call(Pl_Find_Atom("move_star"), 3, args);
	Pl_Query_End(PL_RECOVER);
	return res;
}

void ccs_fprint_process(FILE *fp, proc_t term)
{
int functor, arity;
PlTerm *subterms;

	switch (Type_Of_Term(term)) {
		case ATM: /* Atom */
			fprintf(fp, "%s", Pl_Atom_Name(Rd_Atom_Check(term)));
			break;
		case LST: /* List */
			subterms = Rd_Compound_Check(term, &functor, &arity);
			ccs_fprint_process(fp, subterms[0]);
			term = subterms[1];
			while(Type_Of_Term(term) == LST) {
				subterms = Rd_Compound_Check(term, &functor, &arity);
				fprintf(fp, ",");
				ccs_fprint_process(fp, subterms[0]);
				term = subterms[1];
			}
			break;
		case STC: /* Structure */
			subterms = Rd_Compound_Check(term, &functor, &arity);
			if(!strcmp(Pl_Atom_Name(functor), "pre") && arity == 2) {
				ccs_fprint_process(fp, subterms[0]);
				fprintf(fp, ".");
				if(Type_Of_Term(subterms[1]) == STC) {
					int subfunctor, subarity;

					Rd_Compound_Check(subterms[1], &subfunctor, &subarity);
					if(strcmp(Pl_Atom_Name(subfunctor), "pre") && strcmp(Pl_Atom_Name(subfunctor), "proc")) {
						fprintf(fp, "(");
						ccs_fprint_process(fp, subterms[1]);
						fprintf(fp, ")");
					} 
					else
						ccs_fprint_process(fp, subterms[1]);
				} 
				else
					ccs_fprint_process(fp, subterms[1]);
			} 
			else if(!strcmp(Pl_Atom_Name(functor), "sum") && arity == 2) {
				ccs_fprint_process(fp, subterms[0]);
				fprintf(fp, " + ");
				ccs_fprint_process(fp, subterms[1]);
			} 
			else if(!strcmp(Pl_Atom_Name(functor), "com") && arity == 2) {
				ccs_fprint_process(fp, subterms[0]);
				fprintf(fp, " | ");
				ccs_fprint_process(fp, subterms[1]);
			} 
			else if(!strcmp(Pl_Atom_Name(functor), "res") && arity == 2) {
				fprintf(fp, "(");
				ccs_fprint_process(fp, subterms[0]);
				/* !!! */
				fprintf(fp, ")\\%s{", (fp == stdout ? "" : "\\"));
				ccs_fprint_process(fp, subterms[1]);
				fprintf(fp, "}");
			} 
			else if(!strcmp(Pl_Atom_Name(functor), "in") && arity == 1) {
				ccs_fprint_process(fp, subterms[0]);
			} 
			else if(!strcmp(Pl_Atom_Name(functor), "out") && arity == 1) {
				fprintf(fp, "~");
				ccs_fprint_process(fp, subterms[0]);
			} 
			else if(!strcmp(Pl_Atom_Name(functor), "proc") && arity == 1) {
				ccs_fprint_process(fp, subterms[0]);
			} 
			else if(!strcmp(Pl_Atom_Name(functor), "relabel") && arity == 2) {
				fprintf(fp, "(");
				ccs_fprint_process(fp, subterms[0]);
				fprintf(fp, ")");
				fprintf(fp, "[");
				ccs_fprint_process(fp, subterms[1]);
				fprintf(fp, "]");
			} 
			else if(!strcmp(Pl_Atom_Name(functor), "subst_act") && arity == 2) {
				ccs_fprint_process(fp, subterms[0]);
				fprintf(fp, "/");
				ccs_fprint_process(fp, subterms[1]);
			} 
			else {
				ccs_fprint_plterm(fp, term);
			}
			break;
		default:
		case PLV: /* Prolog variable */
		case FDV: /* Finite domain variable */
		case INT: /* Integer */
		case FLT: /* Floating point number */
			ccs_fprint_plterm(fp, term);
			break;
	}
}

static void ccs_fprint_plterm(FILE *fp, PlTerm term)
{
int i, functor, arity;
PlTerm *subterms;

	switch (Type_Of_Term(term)) {
		case PLV: /* Prolog variable */
			fprintf(fp, "#PLV");
			break;
		case FDV: /* Finite domain variable */
			fprintf(fp, "#FDV");
			break;
		case INT: /* Integer */
			fprintf(fp, "#INT");
			break;
		case FLT: /* Floating point number */
			fprintf(fp, "#FLT");
			break;
		case ATM: /* Atom */
			fprintf(fp, "%s", Pl_Atom_Name(Rd_Atom_Check(term)));
			break;
		case LST: /* List */
			fprintf(fp, "[");
			subterms = Rd_Compound_Check(term, &functor, &arity);
			ccs_fprint_plterm(fp, subterms[0]);
			term = subterms[1];
			while(Type_Of_Term(term) == LST) {
				subterms = Rd_Compound_Check(term, &functor, &arity);
				fprintf(fp, ", ");
				ccs_fprint_plterm(fp, subterms[0]);
				term = subterms[1];
			}
			fprintf(fp, "]");
			break;
		case STC: /* Structure */
			subterms = Rd_Callable_Check(term, &functor, &arity);
			fprintf(fp, "%s(", Pl_Atom_Name(functor));
			if(arity > 0) {
				ccs_fprint_plterm(fp, subterms[0]);
				for(i = 1; i < arity; i++) {
					fprintf(fp, ", ");
					ccs_fprint_plterm(fp, subterms[i]);
				}
				fprintf(fp, ")");
			}
			break;
		default:
			fprintf(fp, "#UNKNOWN\n");
			break;
	}
}

