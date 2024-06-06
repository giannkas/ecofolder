import sys
from sys import stdin
import fileinput

# eg. python3 ids-labels_rules.py examples/oursin_tasmanie/tasmanian_perturb_final.rr 
# examples/oursin_tasmanie/tasmanian_perturb_final_pr_unf.dot

def ids_labels():
  ids_labels.__doc__ = f"""
    {ids_labels.__name__} changes labels to identify a rule using rr file as a source.\n
    Changes will be printed in the dot file
    Usage: python3 {ids_labels.__name__} [options]

    Options:
        -ids    Id names will be printed instead of rule names.
        And viceversa if not given.\n
  """

  ids = 0
  src_rr, src_dot = "", ""
  params = len(sys.argv)

  if params < 3:
    raise ValueError(ids_labels.__doc__)
  elif params <= 4:
    for i in range(params):
      if sys.argv[i] == "-ids" or sys.argv[i] == "--rules":
        ids = 1
      elif ".rr" in sys.argv[i]:
        src_rr = sys.argv[i]
      elif ".dot" in sys.argv[i]:
        src_dot = sys.argv[i]
  
  dids = {}
  drls = {}
  nrls = 0
  frr = open(src_rr)

  for line in frr:
    if "rules:" == line.strip(): nrls = 1
    if nrls >= 1 and ">>" in line:
      rule = line.split('#')[0].strip()
      dids[f"R{nrls}"] = rule
      nrls += 1

  # drls = {value: key for key, value in dids.items()}
  # print(dids)
  # print(drls)

  with open(src_dot, 'r') as fdot:
    content = fdot.read()

  cntl = content.split('\n')

  for l in range(len(cntl)):
    if len(cntl[l]) > 0 and cntl[l].strip()[0] == 'e' and "label=\"" in cntl[l] and "⊥" not in cntl[l]:
      lblp = cntl[l].find("label=\"") + len("label=\"")
      dmp = cntl[l][lblp:].find("\"")
      #print(cntl[l])
      if "(" in cntl[l][lblp:]:
        rl = cntl[l][lblp:cntl[l].find("(")-1]
      else: rl = cntl[l][lblp:lblp+dmp]
      if not ids:
        cntl[l] = cntl[l][:lblp] + dids[rl] + cntl[l][lblp+dmp:]
      else:
        cntl[l] = cntl[l][:lblp] + rl + cntl[l][lblp+dmp:]

    print(cntl[l])
  
  frr.close()


ids_labels()