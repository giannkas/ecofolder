#!/usr/bin/env python

import os
import time
import argparse
from doomed import asp_of_mci
import clingo

def prefix_has_marking(prefix_asp, markings, clingo_opts):
  """
  Checks if the prefix has a reachable marking using clingo's ASP solver.
  
  Args:
    prefix_mci (str): binary representation of the prefix.
    marking (list): List of markings to check.
    clingo_opts (list): List of options for clingo.
  
  Returns:
    bool: True if satisfiable, False otherwise.
  """
  sat = clingo.Control(["1"] + clingo_opts)
  sat.add("base", [], prefix_asp)
  
  for i, m in enumerate(markings):
    #print("".join(["q({},\"{}\").".format(i, p) for p in m]))
    sat.add("base", [], "".join(["q({},\"{}\").".format(i, p) for p in m]))

  sat.add("base", [], """
    conflict(E,F) :- edge(C,E),edge(C,F),E != F, not cutoff(E), not cutoff(F).
    e(F) :- edge(F,C),edge(C,E),e(E).
    :- conflict(E,F), e(E), e(F).
  """)
  
  sat.add("base", [], "{ e(E): event(E), not cutoff(E)}.")
  
  sat.add("base", [], """
    c(C) :- e(E), edge(E,C).
    c(C) :- e(E), edge(C,E).
    c(C) :- h(C,_), not edge(_,C), not e(E): edge(C,E).
    cut(C) :- c(C), not e(E): edge(C,E).
    ncut(A) :- cut(C), h(C,P), name(P,A).
    hcut(C,P) :- cut(C), h(C,P).
  """)
  
  sat.add("base", [], """
    bad(I) :- q(I,_), ncut(P), not q(I,P).
    bad(I) :- not ncut(P), q(I,P).
    ok :- q(I,_), not bad(I).
    :- not ok.
  """)
  
  sat.ground([("base", ())])
  return sat.solve().satisfiable
  #return sat.solve(on_model=print)


if __name__ == "__main__":
  # Set up argument parser
  parser = argparse.ArgumentParser(description="Check if a Petri net model (in .mci format) has a reachable marking using clingo ASP solver.")
  
  # Add arguments
  parser.add_argument("model_unf", help="Path to the input .mci file (unfolding prefix)")
  parser.add_argument("query_marking", help="Comma-separated list of places to query as marking")
  parser.add_argument("--clingo_opts", nargs='*', default=["-W", "none"], help="Options for clingo solver (default: ['-W', 'none'])")
  
  # Parse the arguments
  args = parser.parse_args()
  
  # Extract command-line arguments
  model_unf = args.model_unf
  query_marking = [args.query_marking.split(',')]
  clingo_opts = args.clingo_opts

  # Generate the ASP prefix from the .mci file
  prefix = asp_of_mci(model_unf)

  # Prepare output file path
  base_output = os.path.basename(model_unf.replace(".mci", ""))
  out_d = os.path.split(model_unf)[0]

  # Check if ASP file exists, if not, create one
  if not os.path.exists(f"{out_d}/{base_output}.asp"):
      with open(f"{out_d}/{base_output}.asp", "w") as fp:
          fp.write(prefix)

  # Measure time and perform marking check
  t0 = time.time()
  print(prefix_has_marking(prefix, query_marking, clingo_opts))
