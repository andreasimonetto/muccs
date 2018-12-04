**muCCS** - &mu;framework for Milner's Calculus of Communicating Systems
=====================================================================

```
_______________________________
(      muCCS - version 2.0      )
( by Simonetto, Perfetti, Qorri )
-------------------------------
			 o   ^__^
				o  (oo)\_______
					 (__)\       )\/
							 ||----w |
							 ||     ||
```

This assignment is an effort to develop a program using a programming language
that is based on a logic or symbolic approach (like Scheme, Lisp, Prolog...)
as detailed in our Professor's Guidelines.
The program satisfies the following requirements:

- defines an appropriate structure for states and transitions;
- provides an appropriate representation of the SOS rules of the CCS;
- tests whether a transition is derivable from the SOS rules;
- given a state/process calculates all the transitions exiting from that state;
- given two states S1 and S2, verifies if S2 can reach S1;
- given a state S, determines its LTS associated.

Our choice of using Prolog as a programming language over any other language
was based on the various advantages Prolog has as a Logic Programming Language,
and over its compiler GNU-Prolog. First of all, we choose Prolog because of
its essential incorporated mechanism of backtracking (the mechanism for finding
multiple solutions). Second, we chose Prolog due to the fact that its syntax
and semantic rules are based on Horn's clauses which facilitate the
implementation of such rules.
Last but not least, we chose Prolog as it turned out to be a very interesting
and challenging learning experience of a new programming language.

The idea behind the implementation of the entire project is based on the work
of Gordon D. Plotkin and especially to his article "The Origins of Structural
Operational Semantics". This article guided our work, taught us new concepts
on Structural Operational Semantics and also helped us understand the necessity
of having syntax-directed rules to build this programming language.

Prerequisites
-------------
- A Linux distro (maybe not?)
- GNU Prolog (>=1.4.5) http://gprolog.org/#download
  - Download, extract, chdir
  - ./configure && make && sudo make install
- GNU readline development library https://tiswww.case.edu/php/chet/readline/rltop.html
  - Usually in your distro, packaged as *libreadline-dev* or *libreadline-devel*
- Graphviz (optional but recommended) http://graphviz.org/
  - Hopefully in your distro, packaged as *graphviz*

Compile
-------
To compile, enter the project directory, type "make" and cross your fingers.

Examples
--------

### Vending machine

```
agent VM = coin50.VM50 + coin1.VM1
agent VM50 = coin50.VM1 + coin1.VM150 + ~coffee.VM
agent VM1 = coin50.VM150 + coin1.VM2 + ~coffee.VM50 + ~tea.VM
agent VM150 = coin50.VM2 + ~coffee.VM1 + ~tea.VM50
agent VM2 = ~coffee.VM150 + ~tea.VM1

graphlts VM ; vm.svg
```
![Vending machine LTS](https://raw.github.com/andreasimonetto/muccs/master/tests/vm.png)

### Mutex

```
agent User = ~p.enter.exit.~v.User
agent Sem = p.v.Sem

graphlts (User|Sem|User)\{p,v} ; mutex.svg
```

![Mutex LTS](https://raw.github.com/andreasimonetto/muccs/master/tests/mutex.png)
