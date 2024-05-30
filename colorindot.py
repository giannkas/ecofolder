import sys
from sys import stdin

# eg. python3 colorindot.py "a,b,c;d;e,f,g,h" "red;blue;yellow" 
#  < examples/oursin_tasmanie/tasmanian_perturb3_pr_unf.dot

def color_in_dot():
  color_in_dot.__doc__ = f"""
    {color_in_dot.__name__} changes colors to display and render using dot language.\n
    Usage: python3 {color_in_dot.__name__} [options]\n

    Options:
        -b --blank     enables to fill with white color for rules or events not given as an argument.
        -e --events   assumes that the list of rules are events of a configuration.
        -ru --rules <rule names>    list of rules given <rule names>.
        -co --colors <list of colors>    list of colors given <list of colors>.
        -cocs --color_cutofss <color>    color for cutoff events.\n
    
    File is given by STDIN. If options are not given then {color_in_dot.__name__} will produce the same file.\n
  """

  blank = 0
  evs = 0
  rules, colors, colorcut = "", "", "cornflowerblue"
  params = len(sys.argv)

  for i in range(params):
    if sys.argv[i] == "-b" or sys.argv[i] == "--blank":
      blank = 1
    elif sys.argv[i] == "-e" or sys.argv[i] == "--events":
      evs = 1
    elif sys.argv[i] == "-ru" or sys.argv[i] == "--rules":
      i += 1
      if (i == params or '-' == sys.argv[i][0]): raise ValueError(color_in_dot.__doc__)
      rules = sys.argv[i]
    elif sys.argv[i] == "-co" or sys.argv[i] == "--colors":
      i += 1
      if (i == params or '-' == sys.argv[i][0]): raise ValueError(color_in_dot.__doc__)
      colors = sys.argv[i]
    elif sys.argv[i] == "-cocs" or sys.argv[i] == "--color_cutoffs":
      i += 1
      if (i == params or '-' == sys.argv[i][0]): raise ValueError(color_in_dot.__doc__)
      colorcut = sys.argv[i]

  groups = rules.split(';')
  lcolors = colors.split(';')

  if len(groups) == len(lcolors):
    groups = [ri.split(',') for ri in groups]
    color_rule = {}
    for i in range(len(groups)):
      for j in range(len(groups[i])):
        color_rule[groups[i][j]] = lcolors[i]
  else:
    raise ValueError("Length of rules and colors must be the same.")

  line = stdin.readline()
  while line != "":
    if "fillcolor" in line and "⊥" not in line:
      pos_label = line.find("label=\"") + len("label=\"")
      pos_fillcolor = line.find("fillcolor=")
      sz_fillcolor = len(line[pos_fillcolor:].split()[0])
      fevp = line.find("(")+1
      levp = line.find(")")
      node_name = line[pos_label:fevp-2] if evs == 0 else line[fevp:levp]
      if node_name in color_rule.keys():
        colon = line.find(":")
        fcolor = line[pos_fillcolor+len("fillcolor=\""):colon]
        scolor = line[colon+1:pos_fillcolor+sz_fillcolor-1]
        if fcolor == scolor:
          print(line[:pos_fillcolor+len("fillcolor=")] + "\"" + color_rule[node_name]
              + ":" + color_rule[node_name] + "\"" + 
              line[pos_fillcolor+sz_fillcolor:-1])
        else:
          print(line[:pos_fillcolor+len("fillcolor=")] + "\"" + colorcut
              + ":" + color_rule[node_name] + "\"" + 
              line[pos_fillcolor+sz_fillcolor:-1])
      elif blank == 1:
        if ":" in line:
          print(line[:pos_fillcolor+len("fillcolor=")] + "\"" + "white"
              + ":" + "white" + "\"" + 
              line[pos_fillcolor+sz_fillcolor:-1])
        else:
          print(line[:pos_fillcolor+len("fillcolor=")] + "\"" +
              "white" + "\"" + 
              line[pos_fillcolor+sz_fillcolor:-1])

    else: print(line[:-1])
    line = stdin.readline()

color_in_dot()