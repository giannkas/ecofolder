greater(E,F) :- edge(E,C), edge(C,F), event(E), event(F).
%conflict(E,F) :- event(E), event(F), edge(C,E),edge(C,F),E < F, not cutoff(E), not cutoff(F).
unchallenged(E) :- event(E); E=F : edge(C,E),edge(C,F).
#show greater/2.
%#show conflict/2.
#show unchallenged/1.
#show e2tr(E,N): event(E), h(E,T), name(T,N).
