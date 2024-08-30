#include "anycfg.asp".
#include "configuration.asp".
#include "cut.asp".

% e = C2

#external  my(E) : event(E), E=(pi0,_).

% C subset C1 and C2
e(E) :- my(E).
e(c1,E) :- my(E).

% C2 not in F2
:- c(C),h(C,P),name(P,A): bad(A).

% C1 subset C2 \cap PI0
1 { e(c1,E): e(E), E=(pi0,_) }.
:- e(c1,E), edge(F,C), edge(C,E), not e(c1,F).

c(X,C) :- e(X,E), edge(E,C).
c(X,C) :- e(X,E), edge(C,E).
c(X,C) :- e(X,_), h(C,_), not edge(_,C), not e(X,E): edge(C,E).
cut(X,C) :- c(X,C), not e(X,E): edge(C,E).
ncut(X,A) :- cut(X,C), h(C,P), name(P,A).

% Mark(C1) = Mark(C2)
:- ncut(A), not ncut(c1,A).
:- ncut(c1,A), not ncut(A).

ext(c1,E) :-  event(E), not e(c1,E) ; c(c1,C) : edge(C,E) ; not conflict(F,E) : e(c1,F).

% at least one extension of C has a spoiler in C2
:-  ext(c1,E), edge(C,E), edge(C,F), e(F), E != F.

#show e/2.
#show e/1.
#show ncut/1.
#show ext/2.
