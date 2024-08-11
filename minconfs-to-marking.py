#!/usr/bin/env python
# same result as in all_confs-to-marking.py but using Clingo/ASP.

import os
import sys
import time
from doomed import asp_of_mci,cfg_from_atoms

import clingo

def minconfs(prefix_asp, markings):
  sat = clingo.Control(["--models=0", "--opt-mode=optN", "--heuristic=Domain",
      "--enum-mode=domRec", "--dom-mod=5,16"]+clingo_opts)
  sat.add("base", [], prefix_asp)
  for i, m in enumerate(markings):
    print("".join(["q({},\"{}\").".format(i,p) for p in m]))
    sat.add("base", [], "".join(["q({},\"{}\").".format(i,p) for p in m]))
  sat.add("base", [],
      "conflict(E,F) :- edge(C,E),edge(C,F),E != F."
      "e(F) :- edge(F,C),edge(C,E),e(E)."
      ":- conflict(E,F), e(E), e(F).")
  sat.add("base", [],
      "1 { e(E): event(E) }."
      "#show e/1.")
  sat.add("base", [],
      "c(C) :- e(E), edge(E,C)."
      "c(C) :- e(E), edge(C,E)."
      "c(C) :- h(C,_), not edge(_,C), not e(E): edge(C,E)."
      "cut(C) :- c(C), not e(E): edge(C,E)."
      "ncut(A) :- cut(C), h(C,P), name(P,A)."
      "hcut(C,P) :- cut(C), h(C,P).")
  sat.add("base", [],
      "bad(I) :- q(I,_), ncut(P), not q(I,P)."
      "bad(I) :- not ncut(P), q(I,P)."
      "ok :- q(I,_), not bad(I)."
      ":- not ok.")
  sat.ground([("base",())])
  
  for sol in sat.solve(yield_=True):
    atoms = sol.symbols(atoms=True)
    cfg = cfg_from_atoms(atoms)
    yield cfg

def sort_by_number(string):
  number = int(string.split(',')[-1][1:].strip(')'))
  return number

model_unf = sys.argv[1]
query_marking = [sys.argv[2].split(',')]
prefix = asp_of_mci(model_unf)

base_output = os.path.basename(model_unf.replace(".mci", ""))
out_d = f"{os.path.split(model_unf)[0]}"

if not os.path.exists(f"{out_d}/{base_output}.asp"):
  with open(f"{out_d}/{base_output}.asp", "w") as fp:
    fp.write(prefix)

clingo_opts = ["-W", "none"]

t0 = time.time()
for C in minconfs(prefix, query_marking):
  print(sorted(C, key=sort_by_number))
