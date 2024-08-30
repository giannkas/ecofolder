#!/usr/bin/env python
# same result as in all_confs-to-marking.py but using Clingo/ASP.

import os
import sys
import time
from doomed import asp_of_mci,cfg_from_atoms

from tqdm import tqdm

import clingo

script_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(script_dir)

def minconfs(prefix_asp, markings, shortest=0):

  sat = clingo.Control(["--models=0", "--opt-mode=optN", "--heuristic=Domain",
      "--enum-mode=domRec", "--dom-mod=5,16"]+clingo_opts)
  sat.add("base", [], prefix_asp)
  for i, m in enumerate(markings):
    # print("".join(["q({},\"{}\").".format(i,p) for p in m]))
    sat.add("base", [], "".join(["q({},\"{}\").".format(i,p) for p in m]))
  sat.add("base", [],
      "conflict(E,F) :- edge(C,E),edge(C,F),E != F."
      "e(F) :- edge(F,C),edge(C,E),e(E)."
      ":- conflict(E,F), e(E), e(F).")
  if shortest:
    sat.add("base", [],
        "1 { e(E): event(E) }."
        ":~ e(E). [1@1, E]"
        "#show e/1.")
  else:
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
    if shortest and sol.optimality_proven:
      yield cfg
    else:
      yield cfg

def sort_by_number(string):
  number = int(string.split(',')[-1][1:].strip(')'))
  return number

def str_conf(C):
  Cstr = ""
  for s in C:
    Cstr = Cstr + "".join(s[s.find('e')+1:s.find(')')]) + ","
  Cstr = Cstr[:-1]
  Clist = Cstr.split(',')
  if not '' in Clist: Clist.sort(key=int)
  Cstr = ' '.join(Clist) + ' 0'
  return Cstr

def compute_minconfs():
  compute_minconfs.__doc__ = f"""
    {compute_minconfs.__name__} outputs the minimal/optimal configurations that lead to a marking.\n
    Usage: python3 {compute_minconfs.__name__} [files] [options]\n

    Files:
      -prx --prefix <mcifile>     prefix in .mci format from where the configurations will be computed.
      -mrk --marking <markingsfile>    queried marking to where configurations lead to.

    Options:
      -sht --shortest   mode to select the configurations that contain the least number of events.
      -pdf    mode to render a PDF file to display the configurations.\n
          
    Files are mandatory to compute the configurations and options can be interchangeable to produce different outputs.\n
    """
  
  shortest = 0
  outpdf = 0

  params = len(sys.argv)

  for i in range(params):
    if sys.argv[i] == "-prx" or sys.argv[i] == "--prefix":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
          raise ValueError(compute_minconfs.__doc__)
      model_unf = sys.argv[i]
    elif sys.argv[i] == "-mrk" or sys.argv[i] == "--marking":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
          raise ValueError(compute_minconfs.__doc__)
      query_marking = [sys.argv[i].split(',')]
    elif sys.argv[i] == "-sht" or sys.argv[i] == "--shortest":
      shortest = 1
    elif sys.argv[i] == "-pdf":
      outpdf = 1
  
  prefix = asp_of_mci(model_unf)

  base_output = os.path.basename(model_unf.replace(".mci", ""))
  out_d = f"{os.path.split(model_unf)[0]}"

  if not os.path.exists(f"{out_d}/{base_output}.asp"):
    with open(f"{out_d}/{base_output}.asp", "w") as fp:
      fp.write(prefix)

  clingo_opts = ["-W", "none"]
  t0 = time.time()

  outf = f"{os.path.dirname(model_unf)}/minconfs-to-marking_{base_output}.evev"

  with open(outf, "w") as fout:
    for C in tqdm(minconfs(prefix, query_marking, shortest), desc="Computing minimal configurations"):
      #print(sorted(C, key=sort_by_number))
      print(str_conf(sorted(C, key=sort_by_number)), file=fout)




