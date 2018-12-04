/* ccs.l - CCS Scanner
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

%option prefix="ccs_"
%option noyywrap
%option bison-bridge bison-locations

%{
#include <stdio.h>
#include <string.h>
#include "libccs.h"
#include "ccs_parse.h"

/* Functions to manipulate start_token */
void ccs_parse_transition();
void ccs_parse_definition();
void ccs_parse_process();
%}
	int start_token = 0; 
%%

%{
	/* Here we check if we're parsing a normal CCS process,
	a process definition, a transition list or anything else 
	defined. Once done, the proper token is returned and 
	start_token is reset to default value */
	if(start_token)
	{
	int t = start_token;

		start_token = 0;
		return t;
	}
%}

0|nil	{ return NIL; }
tau	{ return TAU; }

[A-Z][a-zA-Z0-9_]*	{ yylval->str_exp = strdup(yytext); return PID; }
[a-z][a-zA-Z0-9_]*	{ yylval->str_exp = strdup(yytext); return OBS_ACT; }

\~	{ return OP_OUT; }
\.	{ return OP_PREFIX; }
\+	{ return OP_SUM; }
\|	{ return OP_COM; }
\\	{ return OP_RESTRICT; }
\=	{ return OP_DEF; }
\,	{ return COMMA; }
\/	{ return SLASH; }
\(	{ return ROUND_BRACKET_OPEN; }
\)	{ return ROUND_BRACKET_CLOSE; }
\[	{ return SQUARE_BRACKET_OPEN; }
\]	{ return SQUARE_BRACKET_CLOSE; }
\{	{ return BRACE_BRACKET_OPEN; }
\}	{ return BRACE_BRACKET_CLOSE; }

[ \t\r\n]+	{ /* eat up spaces and newlines */ }

%%

void ccs_parse_process() {
	start_token = 0; /* Default */
}

void ccs_parse_transition() {
	start_token = START_TRANSITION;
}

void ccs_parse_definition() {
	start_token = START_DEFINITION;
}

