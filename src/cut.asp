%cut(C) :- e(E),edge(E,C),edge(C,F),not e(F).
%:- cut(C),edge(C,E),e(E).

%cut(C) :- h(C,_), not edge(_,C), not e(E): edge(C,E).
%cut(C) :- e(E), edge(E,C), not e(F): edge(C,F).

c(C) :- e(E), edge(E,C).
c(C) :- e(E), edge(C,E).
c(C) :- h(C,_), not edge(_,C), not e(E): edge(C,E).
cut(C) :- c(C), not e(E): edge(C,E).
ncut(A) :- cut(C), h(C,P), name(P,A).
hcut(C,P) :- cut(C), h(C,P).
