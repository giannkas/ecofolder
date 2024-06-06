import sys
import re
import subprocess


evev_path = sys.argv[1]
dot_file = sys.argv[2]
color_list = sys.argv[3]
seed_number = int(sys.argv[4])


# Read 'evev' file
with open(evev_path, "r") as file:
  evev_lines = file.readlines()

# Read 'tasmanian_perturb_final_pr_unf_colored.dot' file
with open(dot_file, "r") as file:
  dot_lines = file.readlines()


# Extract rule ids and event ids from the dot file
event_rule_mapping = {}
for line in dot_lines:
  match = re.search(r'label="R(\d+) \(e(\d+)\)"', line)
  if match:
    rule_id = int(match.group(1))
    event_id = int(match.group(2))
    event_rule_mapping[event_id] = rule_id

# Generate prompts based on the extracted rule and event mappings

prompts = []
outdot = ""

for line_number, line in enumerate(evev_lines, start=1):
  event_ids = [int(e) for e in line.split() if e != '0']
  
  # Initialize the parts of the -ru parameter
  ru_parts = {
    "part1": [],
    "part2": [],
    "part3": [],
    "part4": []
  }
  
  for event_id in event_ids:
    if event_id in event_rule_mapping:
      rule_id = event_rule_mapping[event_id]
      
      if rule_id <= 11:
        ru_parts["part1"].append(f"e{event_id}")
      elif 12 <= rule_id <= 13:
        ru_parts["part2"].append(f"e{event_id}")
      elif 14 <= rule_id <= 16:
        ru_parts["part3"].append(f"e{event_id}")
      elif rule_id >= 17:
        ru_parts["part4"].append(f"e{event_id}")
  
  ru_value = ";".join([",".join(ru_parts[part]) for part in ["part1", "part2", "part3", "part4"]])
  # color_list = #1a9850;#d73027;#fee08b;#8c510a
  outdot = f"examples/oursin_tasmanie/mrk_instances/urc2alg/tasmanian_perturb_final_pr_unf_ins{line_number}_urc2alg_colored.dot"
  #prompt = f'python3 colorindot.py -e -b -ru "{ru_value}" -co "{color_list}" < {dot_file} > {outdot}'
  with open(dot_file, 'r') as source_dot, open(outdot, 'w') as out_dot:
    subprocess.check_call(["python3", "colorindot.py", "-e", "-b",
                           "-ru", ru_value, "-co", color_list],
                          stdin=source_dot, stdout=out_dot)
  #subprocess.check_call(["python3", "colorindot.py", "-e", "-b",
  #  "-ru", f'\"{ru_value}\"', "-co", f'\"{color_list}\"', "<", dot_file, ">", outdot])
  outpdf = outdot.replace(".dot", ".pdf")
  with open(outpdf, 'w') as opdf:
    subprocess.check_call(["dot", "-T", "pdf", f"-Glabel={seed_number}",
                           outdot], stdout=opdf)
  seed_number+=2
  #prompts.append(prompt)

  
