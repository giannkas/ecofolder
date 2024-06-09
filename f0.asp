#include "configuration.asp".
#include "cut.asp".
%{ e(E): event(E), event(F), not conflict(E, F) }.

:- not ncut(P), bad(P).
%:~ edge(C,F,D),edge(C,E,D),e(F),e(E),c(C). [D,E,F,C]


#show e/1.
