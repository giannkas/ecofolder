bind(C,C2) :- hcut(C,P), edge(C2,_), not edge(_,C2), h(C2,P).
#show bind/2.
