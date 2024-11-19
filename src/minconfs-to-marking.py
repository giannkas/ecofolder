#!/usr/bin/env python
# same result as in all_confs-to-marking.py but using Clingo/ASP.

import os
import sys
import time
import subprocess
from doomed import asp_of_mci,cfg_from_atoms,script_path, Model

from tqdm import tqdm

import clingo

script_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(script_dir)

def minconfs(prefix_asp, markings, shortest=0, clingo_opts=""):

  sat = clingo.Control(["--models=0", "--opt-mode=optN", "--heuristic=Domain",
      "--enum-mode=domRec", "--dom-mod=5,16"]+clingo_opts)
  sat.add("base", [], prefix_asp)
  for i, m in enumerate(markings):
    #print("".join(["q({},\"{}\").".format(i,p) for p in m]))
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
    elif not shortest:
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
      -mdl --model <ll_netfile>     model in .ll_net format to unfold and produce a prefix from where the configurations will be computed.
      -mrk --marking <markingsfile>    queried markings to where configurations lead to.
      -out <outfile>    filename to save the output, if not given then it will be saved in the model location.

    Options:
      -sht --shortest   mode to select the configurations that contain the least number of events.
      -p --pathway display pathway structure instead of event structure.
      -r --repeat    <conf_number>   produce a cutout showing only the configuration selected by <conf_number>.
      -pdf    mode to render a PDF file to display the configurations.\n
          
    <ll_netfile> and <markingsfile> are mandatory to compute the configurations and options can be interchangeable to produce different outputs. If <mcifile> and <ll_netfile> are provided, then {compute_minconfs.__name__} will not unfold to model, instead it assumes that <mcifile> is the corresponding prefix for processing.\n
  """
  
  shortest = 0
  outpdf = 0
  repeat = 0
  pathway = 0
  model_ll = ""
  model_unf = ""
  out_fname = ""
  query_marking = ""

  params = len(sys.argv)

  for i in range(params):
    if sys.argv[i] == "-mdl" or sys.argv[i] == "--model":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
        raise ValueError(compute_minconfs.__doc__)
      model_ll = sys.argv[i]
    if sys.argv[i] == "-prx" or sys.argv[i] == "--prefix":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
        raise ValueError(compute_minconfs.__doc__)
      model_unf = sys.argv[i]
    elif sys.argv[i] == "-mrk" or sys.argv[i] == "--marking":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
        raise ValueError(compute_minconfs.__doc__)
      query_marking = sys.argv[i]
    elif sys.argv[i] == "-out":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
        raise ValueError(compute_minconfs.__doc__)
      out_fname = sys.argv[i]
    elif sys.argv[i] == "-sht" or sys.argv[i] == "--shortest":
      shortest = 1
    elif sys.argv[i] == "-p" or sys.argv[i] == "--pathway":
      pathway = 1
    elif sys.argv[i] == "-r" or sys.argv[i] == "--repeat":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
        raise ValueError(compute_minconfs.__doc__)
      repeat = sys.argv[i]
    elif sys.argv[i] == "-pdf":
      outpdf = 1
  
  # If no filenames are provided, then raise an error message
  if len(model_ll) < 1 or len(query_marking) < 1:
    raise ValueError(compute_minconfs.__doc__)
  
  # Process the model, if not prefix is given, then compute it.
  if ".ll_net" in model_ll:
    base_output = os.path.basename(model_ll.replace(".ll_net", ""))
    model_ll = model_ll.replace(".ll_net", "")
  else:
    base_output = os.path.basename(model_ll.replace(".ll", ""))
    model_ll = model_ll.replace(".ll", "")
  
  model = Model(model_ll)
  expnd_query_markings = []

  # Process bad markings - expand wildcards '*' if used in the markings
  with open(query_marking, "r") as qm:
    for l in qm:
      expnd_query_markings = model.expand_markings(l)
  extd_badmarkings = model.get_badmarkings(expnd_query_markings)

  if len(model_unf) < 1:
    if out_fname == "":
      args_unf = [script_path("ecofolder"), model.filename]
    else:
      args_unf = [script_path("ecofolder"), model.filename, "-m", out_fname + "_unf.mci"]
    subprocess.run(args_unf)

    model_unf = model_ll + "_unf.mci" if out_fname == "" else out_fname + "_unf.mci" if "/" in out_fname and len(out_fname) > 1 else f"{os.path.dirname(model_ll)}/{out_fname}_unf.mci"

  prefix = asp_of_mci(model_unf)

  out_d = f"{os.path.split(model_ll)[0]}"

  if not os.path.exists(f"{out_d}/{base_output}.asp"):
    with open(f"{out_d}/{base_output}.asp", "w") as fp:
      fp.write(prefix)

  clingo_opts = ["-W", "none"]
  t0 = time.time()

  outf = f"{os.path.dirname(model_ll)}/minconfs-to-marking_{base_output}.evev" if out_fname == "" else out_fname + ".evev" if "/" in out_fname and len(out_fname) > 1 else f"{os.path.dirname(model_ll)}/{out_fname}.evev"

  with open(outf, "w") as fout:
    for C in tqdm(minconfs(prefix, extd_badmarkings, shortest, clingo_opts), desc="Computing minimal configurations"):
      #print(sorted(C, key=sort_by_number))
      print(str_conf(sorted(C, key=sort_by_number)), file=fout)

  if outpdf:
    print("Converting to dot...")
    outdot = model_unf.replace(".mci", ".dot")
    with open(outdot, 'w') as out_dot:
      args_mci2dot = [script_path("mci2dot_ev"), "-r", f"{repeat}", "-c", model_unf, outf] if pathway == 0 else [script_path("mci2dot_ev"), "-r", f"{repeat}", "-p", "-c", model_unf, outf]
      subprocess.check_call(args_mci2dot, stdout=out_dot)

    print("Producing the PDF...")
    opdf = model_unf.replace(".mci", ".pdf")
    with open(opdf, 'w') as out_pdf:
      args_dotpdf = ["dot", "-T", "pdf", outdot]
      subprocess.check_call(args_dotpdf, stdout=out_pdf)

    print("Displaying the PDF...")
    args_evince = ["evince", opdf]
    subprocess.Popen(args_evince)
    # subprocess.run(args_evince) keeps the process running even if the pdf is already closed.


if __name__ == "__main__":
  compute_minconfs()




