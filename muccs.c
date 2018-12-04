/* muccs.c - Main function and command-line interface implementation
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
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "libccs.h"

/* Max number of arguments for each command */
#define MAXARG 64

/* CLM stands for Command Line Mode */
#define CLM if(interactive_mode)

/* Flags affecting program behaviour */
static int interactive_mode = 0;
static int quiet_mode = 0;

/* Features type (function of two arguments) */
typedef void (*ccs_function_t) (int c_argc, char **c_argv);

/* Prototypes of program features */
void help(int c_argc, char **c_argv);
void insert(int c_argc, char **c_argv);
void graph_lts(int c_argc, char **c_argv);
void print_lts(int c_argc, char **c_argv);
void quit(int c_argc, char **c_argv);
void test_reachable(int c_argc, char **c_argv);
void test_transition(int c_argc, char **c_argv);
void print_moves(int c_argc, char **c_argv);

/* Auxiliary function for right-trimming strings */
char *trim_string(char *str);

/* Command-line prompt */
const char *PROMPT = "Command (or `help [<command>]'): ";

/* Prologue */
const char *PROLOGUE = "\
 _______________________________ \n\
(      muCCS - version 2.0      )\n\
( by Simonetto, Perfetti, Qorri )\n\
 ------------------------------- \n\
        o   ^__^\n\
         o  (oo)\\_______\n\
            (__)\\       )\\/\n\
                ||----w |\n\
                ||     ||\n\
";

/* Array of commands with associated action and description */
static const struct {
	ccs_function_t action;
	char *key;
	char *brief_desc;
	char *desc;
} MENU_OPTIONS[] = {
	{ help, "help", "Show the online help of the commands", 0 },
	{ insert, "agent", "Insert CCS definitions", 
		"\nDefine (or redefine) a CCS process.\n\n"
		" Syntax: agent <PID> = <Proc> [; <PID> = <Proc> [; ...]]\n"
		"Example: agent A = a.0\n" },
	{ graph_lts, "graphlts", "Create the .svg graph of an LTS", 
		"\nLike `lts' but takes an extra argument that is an output .svg file.\n"
		"Build the graphic representation of an LTS.\n\n"
		" Syntax: graphlts <Proc> ; <SVG-filename>\n\n"
		"Example: graphlts a.0|~a.0 ; output.svg\n\n"
		"NOTE: `graphviz' package required.\n" },
	{ print_lts, "lts", "Given a state/process, determine its LTS associated",
		"\nGiven a state, determine its associated Labelled Transition System (LTS).\n\n"
		" Syntax: lts <Proc>\n"
		"Example: lts a.0|~a.0\n" },
	{ ccs_print_all_defs, "print", "Show the current environment", 
		"\nPrint the environment of the defined processes.\n"
		"No arguments required.\n" },
	{ quit, "quit", "Exit from program", 0 },
	{ test_reachable, "reach", "Given two states, verify if one state can reach the other one",
		"\nGiven two states/processes, verify if one state can reach the other one.\n\n"
		" Syntax: reach <Proc> ; <Proc>\n"
		"Example: reach a.0|~a.0 ; 0|0\n"
		" Output: yes\n" },
	{ test_transition, "test", "Transition test", 
		"\nTest whether or not a transition in input is derivable from the SOS rules.\n\n"
		" Syntax: test <Proc> ; <Proc> ; <Transition>\n"
		"Example: test a.0+b.0 ; 0 ; b\n"
		" Output: yes\n" },
	{ print_moves, "trans", "Show the output transitions from a given state/process", 
		"\nGiven a state/process calculate all the transitions from the given state.\n\n"
		" Syntax: trans <Proc>\n"
		"Example: trans a.0+b.0\n"
		" Output: a -> nil\n"
		"         b -> nil\n" },
	{0, 0, 0}
};




int main(int argc, char *argv[])
{
char *command, *tok, *tmp;
ccs_function_t action;
struct stat sb;
int opt, i;

	/* Test whether or not the program is invoked interactively */
	fstat(STDIN_FILENO, &sb);
	if((sb.st_mode & S_IFMT) == S_IFCHR)
		interactive_mode = 1;
	CLM using_history();
	
	/* Set program options */
	while((opt = getopt(argc, argv, "qh")) != -1) {
		switch(opt) {
			case 'q':
				quiet_mode = 1;
				break;
			case 'h':
			default:
				printf("Usage: muccs [-h] [-q]\n\n");
				printf("-h\tPrint this help\n");
				printf("-q\tQuiet mode\n\n");
				exit(0);
				break;		
		}
	}

	/* Start the Prolog engine with SOS rules */
	Start_Prolog(argc, argv);

	/* */
	if(!quiet_mode)
		puts(PROLOGUE);
	
	/* While user doesn't type `quit' or closes stdin */
	do {
		/* Read the command line */
		action = 0;
		if(!(command = readline(interactive_mode ? PROMPT : ""))) {
			CLM printf("quit\n");
			action = quit;
		}
		else {
			if(!interactive_mode)
				puts(command);
			CLM add_history(trim_string(command));
		}

		/* Search for command into the array MENU_OPTIONS */
		tok = strtok_r(command, " ", &tmp);
		for(i = 0; tok && MENU_OPTIONS[i].action && !action; i++) {
			if(!strcmp(tok, MENU_OPTIONS[i].key))
				action = MENU_OPTIONS[i].action;
		}

		if(action)
		{
		int c_argc = 0;
		char *c_argv[MAXARG];

			/* If found, parse the arguments */
			while((tok = strtok_r(NULL, ";", &tmp)) && c_argc < MAXARG)
				c_argv[c_argc++] = strdup(tok);
			free(command);

			if(c_argc < MAXARG) {
				/* Perform the requested action */
				action(c_argc, c_argv);
				for(i = 0; i < c_argc; i++)
					free(c_argv[i]);
			}
			else {
				fprintf(stderr, "Too many arguments!\n");
				help(0, 0);
			}
		}
		else if(tok) {
			fprintf(stderr, "%s: unknown command.\n\n", tok);
			help(0, 0);
		}
	} while(action != quit);

	/* Stop the Prolog Engine and exit */
	Stop_Prolog();
	return 0;
}

void quit(int c_argc, char **c_argv)
{
	/* */
	if(!quiet_mode)
		printf("\nSo long, and thanks for all the fish!\n");
}

void help(int c_argc, char **c_argv)
{
int i, itemid = -1, print_menu = 0;

	if(c_argc > 0) {
		/* Print help on a specific command */
		for(i = 0; MENU_OPTIONS[i].brief_desc && itemid == -1; i++) {
			if(MENU_OPTIONS[i].desc && !strcmp(c_argv[0], MENU_OPTIONS[i].key)) 
				itemid = i;
		}
		
		if(itemid == -1) {
			fprintf(stderr, "No help available for the command `%s'.\n\n", c_argv[0]);
			print_menu = 1;
		}
		else
			puts(MENU_OPTIONS[itemid].desc);
	}
	else
		print_menu = 1;

	if(print_menu) {
		/* Print menu options */
		for(i = 0; MENU_OPTIONS[i].brief_desc; i++)
			printf("%8s  %s\n", MENU_OPTIONS[i].key, MENU_OPTIONS[i].brief_desc);
		printf("\n");
	}
}

void insert(int c_argc, char **c_argv)
{
proc_t p;
char *line;
int i;

	if(!c_argc) {
		line = readline("p> ");
		CLM add_history(line);
		ccs_read(line, &p, CCS_DEFINITION);
	}
	else {
		for(i = 0; i < c_argc; i++)
			ccs_read(c_argv[i], &p, CCS_DEFINITION);
	}
}

void print_moves(int c_argc, char **c_argv)
{
proc_t p;
char *line;

	switch(c_argc) {
		case 0:
			line = readline("p> ");
			CLM add_history(line);

			if(!ccs_read(line, &p, CCS_PROCESS))
				ccs_print_all_moves(p);
			printf("\n");
			break;
		case 1:
			if(!ccs_read(c_argv[0], &p, CCS_PROCESS))
				ccs_print_all_moves(p);
			printf("\n");
			break;
		default:
			fprintf(stderr, "Too many parameters!\n");
			break;
	}
}

void test_reachable(int c_argc, char **c_argv)
{
proc_t p1, p2;
char *line1, *line2;

	switch(c_argc) {
		case 0:
			line1 = readline("p1> ");
			CLM add_history(line1);

			if(!ccs_read(line1, &p1, CCS_PROCESS)) {
				line2 = readline("p2> ");
				CLM add_history(line2);

				if(!ccs_read(line2, &p2, CCS_PROCESS)) 
					printf("%s\n", (ccs_can_reach(p1, p2) ? "yes" : "no"));
			}
			break;
		case 1:
			if(!ccs_read(c_argv[0], &p1, CCS_PROCESS)) {
				line2 = readline("p2> ");
				CLM add_history(line2);

				if(!ccs_read(line2, &p2, CCS_PROCESS))
					printf("%s\n", (ccs_can_reach(p1, p2) ? "yes" : "no"));
			}
			break;
		case 2:
			if(!ccs_read(c_argv[0], &p1, CCS_PROCESS) && !ccs_read(c_argv[1], &p2, CCS_PROCESS)) 
				printf("%s\n", (ccs_can_reach(p1, p2) ? "yes" : "no"));
			break;
		default:
			fprintf(stderr, "Too many parameters!\n");
			break;
	}
}

void test_transition(int c_argc, char **c_argv)
{
proc_t p1, p2, a1;
char *line1, *line2, *act;

	switch(c_argc) {
		case 0:
			line1 = readline("p1> ");
			CLM add_history(line1);

			if(!ccs_read(line1, &p1, CCS_PROCESS)) {
				line2 = readline("p2> ");
				CLM add_history(line2);

				if(!ccs_read(line2, &p2, CCS_PROCESS)) {
					act = readline("act> ");
					CLM add_history(act);

					if(!ccs_read(act, &a1, CCS_TRANSITION)) 
						printf("%s\n", (ccs_test_trans(p1, p2, a1) ? "yes" : "no"));
				}
			}
			break;
		case 1:
			if(!ccs_read(c_argv[0], &p1, CCS_PROCESS)) {
				line2 = readline("p2> ");
				CLM add_history(line2);

				if(!ccs_read(line2, &p2, CCS_PROCESS)) {
					act = readline("act> ");
					CLM add_history(act);

					if(!ccs_read(act, &a1, CCS_TRANSITION)) 
						printf("%s\n", (ccs_test_trans(p1, p2, a1) ? "yes" : "no"));
				}
			}
			break;
		case 2:
			if(!ccs_read(c_argv[0], &p1, CCS_PROCESS) && !ccs_read(c_argv[1], &p2, CCS_PROCESS)) {
				act = readline("act> ");
				CLM add_history(act);

				if(!ccs_read(act, &a1, CCS_TRANSITION)) 
					printf("%s\n", (ccs_test_trans(p1, p2, a1) ? "yes" : "no"));
			}
			break;
		case 3:
			if(!ccs_read(c_argv[0], &p1, CCS_PROCESS) && !ccs_read(c_argv[1], &p2, CCS_PROCESS) && !ccs_read(c_argv[2], &a1, CCS_TRANSITION)) {
				printf("%s\n", (ccs_test_trans(p1, p2, a1) ? "yes" : "no"));
			}
			break;
		default:
			fprintf(stderr, "Too many parameters!\n");
			break;
	}
}

void print_lts(int c_argc, char **c_argv)
{
proc_t p;
char *line;

	switch (c_argc) {
		case 0:
			line = readline("p> ");
			CLM add_history(line);

			if(!ccs_read(line, &p, CCS_PROCESS))
				ccs_print_lts(p);
			break;
		case 1:
			if(!ccs_read(c_argv[0], &p, CCS_PROCESS))
				ccs_print_lts(p);
			break;
		default:
			fprintf(stderr, "Too many parameters!\n");
			break;
	}

}

void graph_lts(int c_argc, char **c_argv)
{
proc_t p;
char *line;

	switch (c_argc) {
		case 0:
			line = readline("p> ");
			CLM add_history(line);

			if(!ccs_read(line, &p, CCS_PROCESS)) {
				line = readline("output> ");
				CLM add_history(line);
				ccs_print_lts_svg(p, line);
			}
			break;
		case 1:
			if(!ccs_read(c_argv[0], &p, CCS_PROCESS)) {
				line = readline("output> ");
				CLM add_history(line);
				ccs_print_lts_svg(p, line);
			}				
			break;
		case 2:
			if(!ccs_read(c_argv[0], &p, CCS_PROCESS)) 
			{
			int i;
			
				/* left trim filename */
				for(i = 0; c_argv[1][i] == ' ' && c_argv[1][i]; i++);
				
				ccs_print_lts_svg(p, c_argv[1] + i);
			}
			break;
		default:
			fprintf(stderr, "Too many parameters!\n");
			break;
	}
}

char *trim_string(char *str)
{
int i = strlen(str) - 1;

	while(i > 0 && isblank(str[i]))
		i--;
	str[i + 1] = '\0';
	return str;
}

