{ e(E): event(E)}.
%1 { e(E): event(E)}.
conflict(E,F) :- edge(C,E),edge(C,F),E != F.

%:- cutoff(E), e(E).
% select parent events
e(F) :- edge(F,C),edge(C,E),e(E).
% conflicts
:- conflict(E,F), e(E), e(F).
%#show e/1.
%:~ e(E). [1@1, E]
