% sos.pl - SOS rules for CCS as Prolog clauses
% Copyright (C) 2009  Simonetto, Perfetti, Qorri
% 
% This program is free software; you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation; either version 2 of the License, or
% (at your option) any later version.
% 
% This program is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU General Public License for more details.
% 
% You should have received a copy of the GNU General Public License along
% with this program; if not, write to the Free Software Foundation, Inc.,
% 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

% Some useful rules
can_com(A1, A2) :- A1=in(B), A2=out(B).
can_com(A1, A2) :- A1=out(B), A2=in(B).

not_in(_, []).
not_in(tau, [_|_]).
not_in(A, [H|R]) :- A=in(B), B\==H, not_in(A, R).
not_in(A, [H|R]) :- A=out(B), B\==H, not_in(A, R).

not_subst_act(_, []).
not_subst_act(A, [H|R]) :- A=in(B), H\=subst_act(_, B), not_subst_act(A, R).
not_subst_act(A, [H|R]) :- A=out(B), H\=subst_act(_, B), not_subst_act(A, R).

subst_act_member(in(A), in(X), L) :- member(subst_act(A, X), L).
subst_act_member(out(A), out(X), L) :- member(subst_act(A, X), L).

% Dynamic environment for defining processes
:- dynamic(proc_def/2).

% Dynamic structures to keep LTS
:- dynamic(lts_state/2).
:- dynamic(lts_trans/3).

% Act
move(pre(A, P), P, A).

% Sum1
move(sum(P1, _), P1_1, A) :- move(P1, P1_1, A).

% Sum2
move(sum(_, P2), P2_1, A) :- move(P2, P2_1, A).

% Com1
move(com(P1, P2), com(P1_1, P2), A) :- move(P1, P1_1, A).

% Com2
move(com(P1, P2), com(P1, P2_1), A) :- move(P2, P2_1, A).

% Com3
move(com(P1, P2), com(P1_1, P2_1), A) :- 
	move(P1, P1_1, A1), move(P2, P2_1, A2), can_com(A1, A2), A=tau.

% Res
move(res(P1, L), res(P2, L), A) :- move(P1, P2, A), not_in(A, L).

% Rel
move(relabel(P1, L), relabel(P1_1, L), A) :- move(P1, P1_1, X), subst_act_member(A, X, L).
move(relabel(P1, L), relabel(P1_1, L), A) :- move(P1, P1_1, A), not_subst_act(A, L).

% Con
move(proc(K), P1, A) :- proc_def(K, P), nonvar(P), move(P, P1, A).

% Multistep derivation
move_star(X, X, []).
move_star(P1, P2, A) :- move(P1, P1_1, A1), move_star(P1_1, P2, A2), A=[A1|A2].

