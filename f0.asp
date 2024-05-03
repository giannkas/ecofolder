#include "configuration.asp".
#include "cut.asp".
%{ e(E): event(E) }.

:- not ncut(P), bad(P).

#show e/1.
