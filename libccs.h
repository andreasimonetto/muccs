/* libccs.h - function for reading/defining/quering CCS terms
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

#ifndef _LIBCCS_H_
#define _LIBCCS_H_
#include <stdio.h>
#include "gprolog.h"

/* Type of the CCS processes */
typedef PlTerm proc_t;

/* Flags for the ccs_read() function */
enum {
	CCS_PROCESS,
	CCS_TRANSITION,
	CCS_DEFINITION
};

#define ccs_print_process(t) ccs_fprint_process(stdout, t)

/* Parse an input buffer and returns a CCS process in p. Returns 0 if parsing succeded. */
int ccs_read(const char *buf, proc_t *p, int ccs_type);

/* Printing functions */
void ccs_fprint_process(FILE *fp, proc_t term);
void ccs_print_all_moves(proc_t p);
void ccs_print_all_defs();
void ccs_print_lts(proc_t p);
void ccs_print_lts_svg(proc_t p, const char *output);

/* Boolean predicates */
int ccs_can_move(proc_t p1, proc_t p2);
int ccs_can_reach(proc_t p1, proc_t p2);
int ccs_test_trans(proc_t p1, proc_t p2, proc_t a1);

#endif

