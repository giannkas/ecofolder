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
  parser.add_argument("-co", "--colors", type=str, help="List of colors corresponding to the rules, separated by semicolons (e.g., 'red;blue;yellow').")
  parser.add_argument("-cocfs", "--color_cutoffs", type=str, default="none", help="Color for cutoff events. Default is 'none'. This makes more sense considering the cutoff criterion in the unfolding algorithm, not for finding a specific marking. Use -cotips instead.")
  parser.add_argument("-cotips", "--color_endings", type=str, default="none", help="Color for ending events. Default is 'none'. It cannot be used with -cocfs option.")
  parser.add_argument("-dotfile", type=str, help="DOT file from where the colors will be updated.")
  parser.add_argument("-evevfile", type=str, default="", help=".evev file format from where ending events to their corresponding configurations will be found. This file should be provided along with -cotips option (and color), otherwise events will not be distingishable.")
  parser.add_argument("-out", type=str, default="", help="DOT file updated with colors changed.")

  args = parser.parse_args()

  if args.rules and args.colors:
    rule_groups = args.rules.split(';')
    color_list = args.colors.split(';')
    if len(rule_groups) != len(color_list):
      parser.error("The number of rule groups and colors must be the same.")
  else:
    rule_groups = color_list = []

  return args, rule_groups, color_list

def build_color_rule_map(rule_groups, color_list):
  """
  Build a dictionary mapping each rule to its corresponding color.
  
  Args:
    rule_groups (list): List of rule groups.
    color_list (list): List of colors.
  
  Returns:
    dict: Mapping of rule to color.
  """
  color_rule = {}
  for group, color in zip(rule_groups, color_list):
    for rule in group.split(','):
      color_rule[rule] = color
  return color_rule

def color_in_dot():
  """
  Modifies the DOT file read from stdin by changing the fillcolor attribute based on rules and colors provided.
  
  Example usage:
    python3 colorindot.py -ru "a,b,c;d;e,f,g,h" -co "red;blue;yellow" < input.dot > output.dot
    
    - This command will read the DOT file from `input.dot`, apply the colors "red", "blue", and "yellow" to the respective rule groups
      ("a,b,c", "d", and "e,f,g,h"), and output the modified DOT content to `output.dot`.
    - Use the `-b` or `--blank` option to fill unspecified rules or events with white color.
    - Use the `-e` or `--events` option to treat the list of rules as events.
    - The `-cocfs` or `--color_cutoffs` option can be used to specify a different color for cutoff events.
    - The `-cotips` or `--color_endings` option can be used to specify a different color for ending events.
  """
  args, rule_groups, color_list = parse_arguments()
  color_rule = build_color_rule_map(rule_groups, color_list)

  evends = []
  if args.evevfile != "":
    with open(args.evevfile, "r") as evevfile:
      for conf in evevfile:
        conf = conf.strip().split(" ")
        conf = conf[-2] if conf[-1] == '0' else conf[-1]
        evends.append(str(conf))

  with open(args.dotfile, "r") as dotfile, open(args.out, "w") as outdot: 
    for line in dotfile:
      if "fillcolor" in line and "⊥" not in line:
        pos_label = line.find("label=\"") + len("label=\"")
        pos_fillcolor = line.find("fillcolor=")
        fevp = line.find("(") + 1
        levp = line.find(")")
        node_name = line[pos_label:fevp-2] if not args.events else line[fevp:levp]
        evid = line[fevp+1:levp]

        if node_name in color_rule:
          end_fillcolor_pos = line.find("\"", pos_fillcolor+len("fillcolor=\""))
          if ":" in line:
            colon = line.find(":")

            first_color = line[pos_fillcolor+len("fillcolor=\""):colon]
            second_color = line[colon+1:end_fillcolor_pos]

            if args.color_cutoffs == "none" and args.color_endings == "none" :
              new_color = f"{color_rule[node_name]}:{color_rule[node_name]}"
            elif args.color_cutoffs != "none" :
              new_color = f"{color_rule[node_name]}:{args.color_cutoffs}" if first_color != second_color else f"{color_rule[node_name]}:{color_rule[node_name]}"
            elif args.color_endings != "none" and args.evevfile != "":
              new_color = f"{color_rule[node_name]}:{args.color_endings}" if evid in evends else f"{color_rule[node_name]}:{color_rule[node_name]}"


          line = line[:pos_fillcolor+len("fillcolor=\"")] + new_color + line[end_fillcolor_pos:]

        elif args.blank:
          if ":" in line:
            new_color = "white:white"
          else:
            new_color = "white"

          line = line[:pos_fillcolor+len("fillcolor=\"")] + new_color + line[end_fillcolor_pos:]

      print(line, end="", file=outdot)

if __name__ == "__main__":
  color_in_dot()