import sys
import argparse
from sys import stdin

def parse_arguments():
  """
  Parse command-line arguments using argparse.
  
  Returns:
    argparse.Namespace: Parsed arguments.
  """
  parser = argparse.ArgumentParser(
    description="Change colors to display and render using DOT language.",
    epilog="If options are not given, the script will produce the same file."
  )

  parser.add_argument("-b", "--blank", action="store_true", help="Fill with white color for rules or events not given as an argument.")
  parser.add_argument("-e", "--events", action="store_true", help="Assume that the list of rules are events of a configuration.")
  parser.add_argument("-ru", "--rules", type=str, help="List of rules separated by semicolons (e.g., 'a,b,c;d;e,f,g,h').")
  parser.add_argument("-pl", "--places", type=str, help="List of places separated by semicolons (e.g., 's,t,u;v;w,x,y,z').")
  parser.add_argument("-co", "--colors", type=str, help="List of colors corresponding to the elements (rules or places), separated by semicolons (e.g., 'red;blue;yellow').")
  parser.add_argument("-cocfs", "--color_cutoffs", type=str, default="none", help="Color for cutoff events. Default is 'none'. This is in line with the cutoff criterion in the unfolding algorithm, not for finding a specific marking. Use -cotips instead.")
  parser.add_argument("-cotips", "--color_endings", type=str, default="none", help="Color for ending events. Default is 'none'. It cannot be used with -cocfs option.")
  parser.add_argument("-dotfile", type=str, help="DOT file from where the colors will be updated.")
  parser.add_argument("-evevfile", type=str, default="", help=".evev file format from where ending events to their corresponding configurations will be found. This file should be provided along with -cotips option (and color), otherwise events will not be distinguishable.")
  parser.add_argument("-crestfile", type=str, default="", help=".crest file format where events in the configuration crest will be found. This file should be provided along with -cotips option (and color), otherwise events will not be distinguishable or ending events will be colored not the crest.")
  parser.add_argument("-out", type=str, default="", help="DOT file updated with colors changed.")

  args = parser.parse_args()

  assert bool(args.rules) ^ bool(args.places), "A list of 'places' xor 'rules' must be provided."

  if args.rules and args.colors:
    groups = args.rules.split(';')
    color_list = args.colors.split(';')
    if len(groups) != len(color_list):
      parser.error("The number of rule groups and colors must be the same.")
  elif args.places and args.colors:
    groups = args.places.split(';')
    color_list = args.colors.split(';')
    if len(groups) != len(color_list):
      parser.error("The number of place groups and colors must be the same.")
  else:
    groups = color_list = []

  return args, groups, color_list

def build_color_elem_map(groups, color_list):
  """
  Build a dictionary mapping each element to its corresponding color.
  
  Args:
    groups (list): List of rule or place groups.
    color_list (list): List of colors.
  
  Returns:
    dict: Mapping of element to color.
  """
  color_elem = {}
  for group, color in zip(groups, color_list):
    for elem in group.split(','):
      color_elem[elem] = color
  return color_elem

def color_in_dot():
  
  args, groups, color_list = parse_arguments()
  color_elem = build_color_elem_map(groups, color_list)

  evends = []
  if args.evevfile != "" and args.crestfile == "":
    with open(args.evevfile, "r") as evevfile:
      for conf in evevfile:
        conf = conf.strip().split(" ")
        conf = conf[-2] if conf[-1] == '0' else conf[-1]
        evends.append(str(conf))
  elif args.evevfile != "" and args.crestfile != "":
    with open(args.crestfile, "r") as crestfile:
      for conf in crestfile:
        conf = conf.strip().split(" ")
        conf = conf[:-1]
        for ev in conf:
          evends.append(str(ev))

  with open(args.dotfile, "r") as dotfile, open(args.out, "w") as outdot: 
    for line in dotfile:
      if "shape=circle" in line:
        pos_label = line.find("label= <") + len("label= <")
        pos_fillcolor = line.find("fillcolor=")
        fplp = line.find("<") + 1
        lplp = line.find("+") if "+" in line else line.find("-") + 1
        node_name = line[fplp:lplp]

        if node_name in color_elem:
          end_fillcolor_pos = line.find("\"", pos_fillcolor+len("fillcolor=\""))
          new_color = f"{color_elem[node_name]}"
          line = line[:pos_fillcolor+len("fillcolor=\"")] + new_color + line[end_fillcolor_pos:]

      elif "shape=box" in line and "⊥" not in line:
        pos_label = line.find("label=\"") + len("label=\"")
        pos_fillcolor = line.find("fillcolor=")
        fevp = line.find("(") + 1
        levp = line.find(")")
        node_name = line[pos_label:fevp-2] if not args.events else line[fevp:levp]
        evid = line[fevp+1:levp]

        if node_name in color_elem:
          end_fillcolor_pos = line.find("\"", pos_fillcolor+len("fillcolor=\""))
          if ":" in line:
            colon = line.find(":")

            first_color = line[pos_fillcolor+len("fillcolor=\""):colon]
            second_color = line[colon+1:end_fillcolor_pos]

            if args.color_cutoffs == "none" and args.color_endings == "none" :
              new_color = f"{color_elem[node_name]}:{color_elem[node_name]}"
            elif args.color_cutoffs != "none" :
              new_color = f"{color_elem[node_name]}:{args.color_cutoffs}" if first_color != second_color else f"{color_elem[node_name]}:{color_elem[node_name]}"
            elif args.color_endings != "none" and args.evevfile != "":
              new_color = f"{color_elem[node_name]}:{args.color_endings}" if evid in evends else f"{color_elem[node_name]}:{color_elem[node_name]}"


          line = line[:pos_fillcolor+len("fillcolor=\"")] + new_color + line[end_fillcolor_pos:]

        elif args.blank:
          if ":" in line:
            new_color = "transparent:transparent"
          else:
            new_color = "transparent"

          line = line[:pos_fillcolor+len("fillcolor=\"")] + new_color + line[end_fillcolor_pos:]

      print(line, end="", file=outdot)

if __name__ == "__main__":
  color_in_dot()