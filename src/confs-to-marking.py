#!/usr/bin/env python

import os
import sys
import time
import subprocess
from doomed import asp_of_mci,h,get_eid,script_path,py_of_symbol,get_event_poset,Model

from tqdm import tqdm

import clingo

script_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(script_dir)

def cfg_from_atoms(atoms):
  return h({get_eid(py_of_symbol(a.arguments[0])) for a in atoms
              if a.name == "e"})

def rdn_from_atoms(atoms):
  return h({get_eid(py_of_symbol(a.arguments[0])) for a in atoms
              if a.name == "redundant"})

def nrdn_from_atoms(atoms):
  return h({get_eid(py_of_symbol(a.arguments[0])) for a in atoms
              if a.name == "e_non_redundant"})

def get_crest(poset):
  return {e for e, od in poset.out_degree() if od == 0}

def minconfs(prefix_asp, markings, shortest=0, redundant=1, clingo_opts=""):

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
  sat.add("base", [],
      "in_h_edge(P,E) :- edge(C,E), h(C,P)."
      "out_h_edge(P,E) :- edge(E,C), h(C,P)."
      "redundant(E) :- e(E); in_h_edge(P,E) : out_h_edge(P,E); out_h_edge(P,E) : in_h_edge(P,E).")
  sat.add("base", [],
      "e_non_redundant(E) :- e(E), not redundant(E).")
      

  sat.ground([("base",())])
  
  # Print all grounded atoms and rules
  # print("=== Grounded Atoms ===")
  # for symbol in sat.symbolic_atoms:
  #   print(symbol.symbol)

  # print("=== Grounded Rules ===")
  # for rule in sat.theory_atoms:
  #   print(rule)

  for sol in sat.solve(yield_=True):
    atoms = sol.symbols(atoms=True)
    cfg = cfg_from_atoms(atoms)
    rdn = rdn_from_atoms(atoms)
    nrdn = nrdn_from_atoms(atoms)
    if rdn and rdn[0] in cfg:
      cfg_lst = list(cfg)
      cfg_lst[cfg.index(rdn[0])] = rdn[0][:-1] + "+" + rdn[0][-1:]
      cfg = tuple(cfg_lst)
    if redundant and shortest and sol.optimality_proven:
      yield cfg
    elif redundant and not shortest:
      yield cfg
    elif not redundant and shortest and sol.optimality_proven:
      yield nrdn
    else:
      yield nrdn

def maxconfs(prefix_asp, bad_aspfile, badmrks=1, redundant=1, clingo_opts=""):
  sat = clingo.Control(["0", "--heuristic=Domain",
      "--enum-mode=domRec", "--dom-mod=3,16"]+clingo_opts)
  sat.add("base", [], prefix_asp)
  sat.load(bad_aspfile)
  sat.load(script_path("configuration.asp"))
  #sat.load(script_path("anycfg.asp"))
  if badmrks > 1:
    tobads = ":- "
    for i in range(1,badmrks+1):
      if i == badmrks:
        tobads += f" not ncut(P), bad{i}(P)."
      else:
        tobads += f"not ncut(P), bad{i}(P);"
    sat.add("base", [], f"#include \"{script_path("configuration.asp")}\"."
             f"#include \"{script_path("cut.asp")}\".")
    sat.add("base", [], tobads)
    sat.add("base", [], "#show e/1.")
  else:
    sat.load(script_path("f0.asp"))

  sat.add("base", [],
      "in_h_edge(P,E) :- edge(C,E), h(C,P)."
      "out_h_edge(P,E) :- edge(E,C), h(C,P)."
      "redundant(E) :- e(E); in_h_edge(P,E) : out_h_edge(P,E); out_h_edge(P,E) : in_h_edge(P,E).")
  sat.add("base", [],
      "e_non_redundant(E) :- e(E), not redundant(E).")
  
  sat.ground([("base",())])
  # for sol in sat.solve(yield_=True):
  #   atoms = sol.symbols(atoms=True)
  #   cfg = cfg_from_atoms(atoms)
  #   yield cfg
  for sol in sat.solve(yield_=True):
    atoms = sol.symbols(atoms=True)
    cfg = cfg_from_atoms(atoms)
    rdn = rdn_from_atoms(atoms)
    nrdn = nrdn_from_atoms(atoms)
    if rdn and rdn[0] in cfg:
      cfg_lst = list(cfg)
      cfg_lst[cfg.index(rdn[0])] = rdn[0][:-1] + "+" + rdn[0][-1:]
      cfg = tuple(cfg_lst)
    if redundant:
      yield cfg
    else:
      yield nrdn

def sort_by_number(string):
  number_str = string.split(',')[-1][1:].strip(')')
  number_str = number_str.rstrip('+')
  number = int(number_str)
  return number

def str_conf(C):
  Cstr = ""
  for s in C:
    Cstr = Cstr + "".join(s[s.find('e')+1:s.find(')')]) + ","
  Cstr = Cstr[:-1]
  Clist = Cstr.split(',')
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
      -nrdn --no-redundant   mode to select the configurations that contain no redundant events, redundant events are marked with '+' sign.
      -mcmillan    collate the mcmillan criterion to unfold the net in ecofolder.
      -maxconf   produce the maximal configurations instead of minimal ones to the specified marking. 
      -p --pathway display pathway structure instead of event structure.
      -r --repeat    <conf_number>   produce a cutout showing only the configuration selected by <conf_number>.
      -pdf    mode to render a PDF file to display the configurations.\n
          
    <ll_netfile> and <markingsfile> are mandatory to compute the configurations and options can be interchangeable to produce different outputs. If <mcifile> and <ll_netfile> are provided, then {compute_minconfs.__name__} will not unfold to model, instead it assumes that <mcifile> is the corresponding prefix for processing.\n
  """
  
  shortest = 0
  redundant = 1
  outpdf = 0
  repeat = 0
  pathway = 0
  model_ll = ""
  model_unf = ""
  out_fname = ""
  query_marking = ""
  mcmillan = 0
  maxconf = 0

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
    elif sys.argv[i] == "-nrdn" or sys.argv[i] == "--no-redundant":
      redundant = 0
    elif sys.argv[i] == "-mcmillan":
      mcmillan = 1
    elif sys.argv[i] == "-maxconf":
      maxconf = 1
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
      expnd_query_markings += model.expand_markings(l)
  extd_badmarkings = model.get_badmarkings(expnd_query_markings)

  if len(model_unf) < 1:
    if out_fname == "":
      args_unf = [script_path("ecofolder"), "-mcmillan" if mcmillan else None, model.filename]
    else:
      args_unf = [script_path("ecofolder"), model.filename, "-mcmillan" if mcmillan else None, "-m", out_fname + "_unf.mci"]
    args_unf = [arg for arg in args_unf if arg is not None]
    subprocess.run(args_unf)

    model_unf = model_ll + "_unf.mci" if out_fname == "" else out_fname + "_unf.mci" if "/" in out_fname and len(out_fname) > 1 else f"{os.path.dirname(model_ll)}/{out_fname}_unf.mci"

  clingo_opts = ["-W", "none"]
  prefix = asp_of_mci(model_unf)
  prefix_info = get_event_poset(prefix, clingo_opts)
  prefix_d = prefix_info["poset"]

  out_d = f"{os.path.split(model_ll)[0]}"

  if not os.path.exists(f"{out_d}/{base_output}.asp"):
    with open(f"{out_d}/{base_output}.asp", "w") as fp:
      fp.write(prefix)

  if maxconf:
    bad_aspfile = os.path.join(out_d, "bad.asp")
    with open(bad_aspfile, "w") as fp:
      if len(extd_badmarkings) > 1:
        curm = 1
        for m in extd_badmarkings:
          for p in m:
            fp.write(f"{clingo.Function(f'bad{curm}', (clingo.String(p),))}.\n")
          curm += 1
      else:
        for m in extd_badmarkings:
          for p in m:
            fp.write(f"{clingo.Function('bad', (clingo.String(p),))}.\n")

  t0 = time.time()

  operation = "maxconfs-to-marking_" if maxconf else "minconfs-to-marking_"

  outf = f"{os.path.dirname(model_ll)}/{operation}{base_output}.evev" if out_fname == "" else out_fname + ".evev" if "/" in out_fname and len(out_fname) > 1 else f"{os.path.dirname(model_ll)}/{out_fname}.evev"
  outf_crest = f"{os.path.dirname(model_ll)}/{operation}{base_output}.crest" if out_fname == "" else out_fname + ".crest" if "/" in out_fname and len(out_fname) > 1 else f"{os.path.dirname(model_ll)}/{out_fname}.crest"

  with open(outf, "w") as fout, open(outf_crest, "w") as fout_crest:
    if maxconf:
      for C in tqdm(maxconfs(prefix, bad_aspfile, len(extd_badmarkings), redundant, clingo_opts), desc="Computing maximal configurations"):
        C_d = prefix_d.subgraph(C)
        C_crest = get_crest(C_d)
        print(str_conf(sorted(C_crest, key=sort_by_number)), file=fout_crest)
        print(str_conf(sorted(C, key=sort_by_number)), file=fout)
    else:
      for C in tqdm(minconfs(prefix, extd_badmarkings, shortest, redundant, clingo_opts), desc="Computing minimal configurations"):
        C_d = prefix_d.subgraph(C)
        C_crest = get_crest(C_d)
        print(str_conf(sorted(C_crest, key=sort_by_number)), file=fout_crest)
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




