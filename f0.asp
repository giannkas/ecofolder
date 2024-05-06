#include "configuration.asp".
#include "cut.asp".
%{ e(E): event(E), event(F), not conflict(E, F) }.

:- not ncut(P), bad(P).

#show e/1.
