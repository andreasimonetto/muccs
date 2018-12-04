/* ccs.y - CCS syntax-directed parser that translate CCS terms in Prolog clauses
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

%{ 
#include <stdio.h>
#include <stdlib.h>
#include "libccs.h"
#define YYERROR_VERBOSE
%}

//%name-prefix="ccs_"
%locations
%pure-parser
%parse-param { PlTerm *root }
%union {
	char *str_exp;
	long pl_term;
}

%{
extern int ccs_lex(YYSTYPE *lvalp, YYLTYPE *llocp);
int ccs_error(YYLTYPE *locp, PlTerm *root, const char *msg);
%}

%token START_TRANSITION START_DEFINITION 
%token <str_exp> PID "process identifier"
%token <str_exp> OBS_ACT "observable action"
%token NIL "nil"
%token TAU "tau"
%token OP_OUT "~"
%token OP_DEF "="
%token OP_PREFIX "."
%token OP_SUM "+"
%token OP_COM "|"
%token OP_RESTRICT "\\"
%token COMMA ","
%token ROUND_BRACKET_OPEN "("
%token ROUND_BRACKET_CLOSE ")"
%token SQUARE_BRACKET_OPEN "["
%token SQUARE_BRACKET_CLOSE "]"
%token BRACE_BRACKET_OPEN "{"
%token BRACE_BRACKET_CLOSE "}"
%token SLASH "/"

%nonassoc OP_RESTRICT
%left OP_PREFIX OP_COM OP_SUM

%type <pl_term> Proc "process"
%type <pl_term> ProcDef "process definition"
%type <pl_term> Rel "relabel"
%type <pl_term> RelList "relabeling list"
%type <pl_term> ObsActList "observable actions list"
%type <pl_term> TransList "transition list"
%start Process

%%
Process
	: Proc {
		*root = $1;
		YYACCEPT;
	}
	| START_TRANSITION TransList {
		*root = $2; 
		YYACCEPT;
	}
	| START_DEFINITION ProcDef {
		*root = $2;
		YYACCEPT;
	}
	;

ProcDef 
	: PID OP_DEF Proc {
		PlTerm args[3];
		
		/* Delete previously defined process PID (if any) */
		args[0] = Mk_String($1);
		args[1] = Mk_Variable();		
		args[2] = Mk_Compound(Pl_Create_Allocate_Atom("proc_def"), 2, args);			
		Pl_Query_Begin(TRUE);
		Pl_Query_Call(Pl_Find_Atom("retract"), 1, args + 2);
		Pl_Query_End(PL_RECOVER);

		/* Insert the new definition */				
		$$ = args[0] = Mk_String($1);
		args[1] = $3;
		args[2] = Mk_Compound(Pl_Create_Allocate_Atom("proc_def"), 2, args);	
		Pl_Query_Begin(TRUE);
		Pl_Query_Call(Pl_Find_Atom("assertz"), 1, args + 2);
		Pl_Query_End(PL_RECOVER);
	}
	;
	
Proc
	: ROUND_BRACKET_OPEN Proc ROUND_BRACKET_CLOSE {
		$$ = $2;
	}
	| NIL {
		$$ = Mk_String("nil");
	}
	| PID {
		PlTerm args[1];
		args[0] = Mk_String($1);
		$$ = Mk_Compound(Pl_Create_Allocate_Atom("proc"), 1, args);
	}
	| TAU OP_PREFIX Proc {
		PlTerm args[2];
		args[0] = Mk_String("tau");
		args[1] = $3;
		$$ = Mk_Compound(Pl_Create_Allocate_Atom("pre"), 2, args);
	}
	| OBS_ACT OP_PREFIX Proc {
		PlTerm args[3];
		args[0] = Mk_String($1);
		args[1] = Mk_Compound(Pl_Create_Allocate_Atom("in"), 1, args);
		args[2] = $3;
		$$ = Pl_Mk_Compound(Pl_Create_Allocate_Atom("pre"), 2, args + 1);
	}
	| OP_OUT OBS_ACT OP_PREFIX Proc {
		PlTerm args[3];
		args[0] = Mk_String($2);
		args[1] = Mk_Compound(Pl_Create_Allocate_Atom("out"), 1, args);
		args[2] = $4;
		$$ = Mk_Compound(Pl_Create_Allocate_Atom("pre"), 2, args + 1);
	}
	| Proc OP_SUM Proc {
		PlTerm args[2];
		args[0] = $1;
		args[1] = $3;
		$$ = Mk_Compound(Pl_Create_Allocate_Atom("sum"), 2, args);
	}
	| Proc OP_COM Proc {
		PlTerm args[2];
		args[0] = $1;
		args[1] = $3;
		$$ = Mk_Compound(Pl_Create_Allocate_Atom("com"), 2, args);
	}
	| Proc OP_RESTRICT BRACE_BRACKET_OPEN ObsActList BRACE_BRACKET_CLOSE {
		PlTerm args[2];
		args[0] = $1;
		args[1] = $4;
		$$ = Mk_Compound(Pl_Create_Allocate_Atom("res"), 2, args);
	}
	| ROUND_BRACKET_OPEN Proc ROUND_BRACKET_CLOSE SQUARE_BRACKET_OPEN RelList SQUARE_BRACKET_CLOSE {
		PlTerm args[2];
		args[0] = $2;
		args[1] = $5;
		$$ = Mk_Compound(Pl_Create_Allocate_Atom("relabel"), 2, args);
	}
	| error {
		yyclearin;
		YYABORT;
	}
	;

ObsActList
	: OBS_ACT {
		PlTerm args[2];
		args[0] = Mk_String($1);
		args[1] = Mk_List(0);
		$$ = Mk_List(args);
	}
	| OBS_ACT COMMA ObsActList {
		PlTerm args[2];
		args[0] = Mk_String($1);
		args[1] = $3;
		$$ = Mk_List(args);
	}
	;

Rel
	: OBS_ACT SLASH OBS_ACT {
		PlTerm args[2];
		args[0] = Mk_String($1);
		args[1] = Mk_String($3);
		$$ = Mk_Compound(Pl_Create_Allocate_Atom("subst_act"), 2, args);
	}
	;

RelList
	: Rel {
		PlTerm args[2];
		args[0] = $1;
		args[1] = Mk_List(0);
		$$ = Mk_List(args);
	}
	| Rel COMMA RelList {
		PlTerm args[2];
		args[0] = $1;
		args[1] = $3;
		$$ = Mk_List(args);
	}
	;

TransList
	: OBS_ACT { 
		PlTerm args[3];
		args[0] = Mk_String($1);
		args[1] = Mk_Compound(Pl_Create_Allocate_Atom("in"), 1, args);
		args[2] = Mk_List(0);
		$$ = Mk_List(args+1);
	}
	| OP_OUT OBS_ACT { 
		PlTerm args[3];
		args[0] = Mk_String($2);
		args[1] = Mk_Compound(Pl_Create_Allocate_Atom("out"), 1, args);
		args[2] = Mk_List(0);
		$$ = Mk_List(args+1);
	}
	| NIL { 
		PlTerm args[2];
		args[0] = Mk_String("nil");
		args[1] = Mk_List(0);
		$$ = Mk_List(args);
	}
	| TAU { 
		PlTerm args[2];
		args[0] = Mk_String("tau");
		args[1] = Mk_List(0);
		$$ = Mk_List(args);
	}
	| OBS_ACT OP_PREFIX TransList {
		PlTerm args[3];
		args[0] = Mk_String($1);
		args[1] = Mk_Compound(Pl_Create_Allocate_Atom("in"), 1, args);		
		args[2] = $3;
		$$ = Mk_List(args + 1);
	}
	| OP_OUT OBS_ACT OP_PREFIX TransList {
		PlTerm args[3];
		args[0] = Mk_String($2);
		args[1] = Mk_Compound(Pl_Create_Allocate_Atom("out"), 1, args);
		args[2] = $4;
		$$ = Mk_List(args + 1);
	}
	| NIL OP_PREFIX TransList {
		PlTerm args[2];
		args[0] = Mk_String("nil");
		args[1] = $3;
		$$ = Mk_List(args);
	}
	| TAU OP_PREFIX TransList {
		PlTerm args[2];
		args[0] = Mk_String("tau");
		args[1] = $3;
		$$ = Mk_List(args);
	}
	| error {
		yyclearin;
		YYABORT;
	}
	;
%%

int ccs_error(YYLTYPE *locp, PlTerm *root, const char *msg)
{
	fprintf(stderr, "%s.\n", msg);
	*root = 0;
	/*YYABORT;*/
	return 0;
}

