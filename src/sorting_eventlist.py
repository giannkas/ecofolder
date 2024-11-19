#!/usr/bin/env python

import os
import sys

script_dir = os.path.dirname(os.path.abspath(__file__))
parent_dir = os.path.dirname(script_dir)

def sort_indepth(dic, maxsize):
  for key in dic:
    if len(dic[key]) <= maxsize:
      pass
    else:
      newdic = {}
      for subconf in dic[key]:
        if subconf[0] in newdic:
          newdic[subconf[0]].append(subconf[1:])
        else:
          newdic[subconf[0]] = [subconf[1:]]
      dic[key] = newdic
      sort_indepth(newdic, maxsize)
  return dic

def depth(dic):
  newdic = {}
  for key in dic:
    if isinstance(dic[key], list):
      newdic[key] = len(dic[key])
    else:
      newdic[key] = depth(dic[key])
  return newdic

def flatten_depth(dic_depths, topkeys = ""):
  newdic_depths = {}
  for key in dic_depths:
    if isinstance(dic_depths[key], int):
      topkeys = topkeys + "," if topkeys != "" and topkeys[-1] != "," else topkeys
      newdic_depths[topkeys + str(key)] = dic_depths[key]
    else:
      topkeys += "," + str(key) if topkeys != "" and topkeys[-1] != "," else str(key) + ","
      newdic_depths = newdic_depths | flatten_depth(dic_depths[key], topkeys)
      topkeys = topkeys.strip(",").split(",")
      topkeys.pop()
      topkeys = ",".join(key for key in topkeys)
  return newdic_depths

def flatten_dic(dic, topkeys = []):
  flat_dic = []
  for key in dic:
    if isinstance(dic[key], list):
      flat_dic = flat_dic + [topkeys + [key] + item for item in dic[key]]
    else:
      topkeys += [key]
      flat_dic = flat_dic + flatten_dic(dic[key], topkeys)
      topkeys.pop()
  return flat_dic

def sorting_confs():
  sorting_confs.__doc__ = f"""
    {sorting_confs.__name__} outputs sorted configurations according to longest common cone.\n
    Usage: python3 {sorting_confs.__name__} [files] [options]\n

    Files:
      -evev --events <evevfile>     events list in .evev format from where the configurations will be sorted. If given before the -rls parameter, then <rulesfile> will be ignored.
      -rls --rules <rulesfile>    rules list in .rules format from where the firing sequences will be sorted. If given before the -evev parameter, then <evevfile> will be ignored.
      -out <outfile>    filename to save the output, if not given then it will be saved in the model location with a "_sorted" as final word in the filename.

    Options:
      -print-dic    print a dictonary containing the number of common rules fired in each configuration/firing sequence, along with the sequence in common.
      -min --min-group-size   minimum group size to create a set of configurations with a common cone.
      -max --max-group-size   maximum group size to create a set of configurations with a common cone. This can be surpassed if the minimum size is not reached. Then, you may find groups of size maxsize + minsize - n (where n is less than minsize).\n
          
  """
  
  minsize = 5
  maxsize = 10
  runsfile = ""
  out_fname = ""
  print_dic = 0

  params = len(sys.argv)

  for i in range(params):
    if sys.argv[i] == "-evev" or sys.argv[i] == "--events":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
        raise ValueError(sorting_confs.__doc__)
      runsfile = sys.argv[i]
    elif sys.argv[i] == "-rls" or sys.argv[i] == "--rules":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
        raise ValueError(sorting_confs.__doc__)
      runsfile = sys.argv[i]
    elif sys.argv[i] == "-min" or sys.argv[i] == "--min-group-size":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
        raise ValueError(sorting_confs.__doc__)
      try:
        int(sys.argv[i])
      except ValueError:
        raise ValueError(f"Parameter {sys.argv[i]} must be an integer")
      minsize = int(sys.argv[i])
    elif sys.argv[i] == "-max" or sys.argv[i] == "--max-group-size":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
        raise ValueError(sorting_confs.__doc__)
      try:
        int(sys.argv[i])
      except ValueError:
        raise ValueError(f"Parameter {sys.argv[i]} must be an integer")
      maxsize = int(sys.argv[i])
    elif sys.argv[i] == "-print-dic":      
      print_dic = 1
    elif sys.argv[i] == "-out":
      i += 1
      if (i == params):
        out_fname = 1
      elif ('-' == sys.argv[i][0]):
        raise ValueError(sorting_confs.__doc__)
      else:
        out_fname = sys.argv[i]

  # read from file
  outerdic = {}
  with open(runsfile, "r") as confs:
    for conf in confs:
      if ".evev" in runsfile:
        lstconf = conf.strip().split(" ")[:-1]
      else:
        lstconf = conf.strip().split(" ")
      if lstconf[0] in outerdic:
        outerdic[lstconf[0]].append(lstconf[1:])
      else:
        outerdic[lstconf[0]] = [lstconf[1:]]

  # adjust groups to maxsize
  outerdic = sort_indepth(outerdic, maxsize)
  #print(outerdic)

  # flat inner dictionaries to be stored in a list of lists
  flat_dic = flatten_dic(outerdic)
  # print(flat_dic)

  # Compute the number of configurations located in each key assigned value.
  dic_depths = depth(outerdic)
  #print(dic_depths)

  # Flat the previous created dictionary so the common cones are used as keys.
  flat_dic_depths = flatten_depth(dic_depths)
  if print_dic: print(flat_dic_depths)

  if isinstance(out_fname, str) and out_fname != "":
    with open(out_fname, "w") as output:
      for conf in flat_dic:
        print(conf,file=output)
  elif isinstance(out_fname, int) and out_fname == 1 and ".evev" in runsfile:
    prefixfile = os.path.basename(runsfile.replace(".evev", ""))
    outf = f"{os.path.dirname(runsfile)}/{prefixfile}_sorted.evev"
  elif isinstance(out_fname, int) and out_fname == 1 and ".rules" in runsfile:
    prefixfile = os.path.basename(runsfile.replace(".rules", ""))
    outf = f"{os.path.dirname(runsfile)}/{prefixfile}_sorted.rules"

  if out_fname == 1:
    with open(outf, "w") as output:
      for conf in flat_dic:
        print(" ".join(conf) + " 0",file=output)


if __name__ == "__main__":
  sorting_confs()




