import argparse

def ids_labels():
  """
  ids_labels changes labels in a DOT file to identify a rule using an RR file as a source. It updates the labels in the DOT file based on the rules in the RR file.

  Usage: python3 ids_labels.py [options] rr_file dot_file

  Options:
    -i, --ids   Print IDs instead of rule names. By default, rule names are printed.

  Example:
    python ids_labels.py examples/oursin_tasmanie/tasmanian_perturb_final.rr examples/oursin_tasmanie/tasmanian_perturb_final_pr_unf.dot
    python ids_labels.py -i examples/oursin_tasmanie/tasmanian_perturb_final.rr examples/oursin_tasmanie/tasmanian_perturb_final_pr_unf.dot
  """

  # Setting up the argument parser
  parser = argparse.ArgumentParser(description="Modify DOT file labels using rule IDs or names from an RR file.")
  parser.add_argument("rr_file", help="The RR file containing the rules.")
  parser.add_argument("dot_file", help="The DOT file to update with the new labels.")
  parser.add_argument("-i", "--ids", action="store_true", help="Print IDs instead of rule names.")
  
  # Parse the arguments
  args = parser.parse_args()

  # Dictionary to hold rule IDs and names
  rule_dict = {}
  rule_count = 0

  # Read the RR file and extract rule IDs and names
  with open(args.rr_file, 'r') as rr_file:
    is_rules_section = False
    for line in rr_file:
      if "rules:" in line.strip():
        is_rules_section = True
        continue
      if is_rules_section and ">>" in line:
        rule_count += 1
        rule_name = line.split('#')[0].strip()
        rule_dict[f"R{rule_count}"] = rule_name

  # Function to find and replace rule IDs or names in the DOT file
  def process_line(line):
    if (line.strip().startswith('e') or line.strip().startswith('r')) and "label=\"" in line and "⊥" not in line:
      label_pos = line.find("label=\"") + len("label=\"")
      end_label_pos = line[label_pos:].find("\"")
      label_content = line[label_pos:label_pos + end_label_pos]
      rule = label_content.split("(")[0].strip() if "(" in label_content else label_content
      #print(label_content)
      
      if not args.ids and rule in rule_dict:
        line = line[:label_pos] + rule_dict[rule] + line[label_pos + end_label_pos:]
      elif args.ids:
        line = line[:label_pos] + rule + line[label_pos + end_label_pos:]
    return line

  # Read and process the DOT file
  with open(args.dot_file, 'r') as dot_file:
    for line in dot_file:
      print(process_line(line.strip()))

if __name__ == "__main__":
  ids_labels()
