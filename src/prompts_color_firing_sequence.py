import sys
import re
import subprocess
import os
from doomed import script_path

evev_path = sys.argv[1]
dot_file = sys.argv[2]
color_list = sys.argv[3]
color_range = sys.argv[4]
outdir = sys.argv[5]
init_number = int(sys.argv[6])

# Read 'evev' file
with open(evev_path, "r") as file:
  evev_lines = file.readlines()

# Read 'tasmanian_perturb_final_pr_unf_colored.dot' file
with open(dot_file, "r") as file:
  dot_lines = file.readlines()

cranges = color_range.split(";")
crranges = [int(r) for r in cranges]
ncolors = len(color_list.split(";"))
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

for line_number, line in enumerate(evev_lines, start=1):
  event_ids = [int(e) if '+' not in e else e for e in line.split() if e != '0']
  
  # Initialize the parts of the -ru parameter
  ru_parts = {f"part{i+1}": [] for i in range(ncolors)}
  
  for event_id in event_ids:
    if event_id in event_rule_mapping:
      rule_id = event_rule_mapping[event_id]
      cr = next((index for index, rgs in 
                      enumerate(crranges) 
                      if rule_id <= rgs or (index == len(crranges)-1 and rule_id > rgs)), -1)
      if cr >= 0: ru_parts[f"part{cr+1}"].append(f"e{event_id}")

    elif '+' in event_id:
      eid = int(event_id[:-1])
      ru_parts[f"part{ncolors}"].append(f"e{eid}")
    
  
  ru_value = ";".join([",".join(ru_parts[part]) for part in ru_parts.keys()])
  outfile = os.path.basename(dot_file.replace(".dot",""))
  outdot = f"{outdir}{outfile}_ins{line_number}.dot"

  with open(dot_file, 'r') as source_dot, open(outdot, 'w') as out_dot:
    subprocess.check_call(["python", script_path("color_in_dot.py"), "-e", "-b",
                           "-ru", ru_value, "-co", color_list, "-cocs", "none" ],
                          stdin=source_dot, stdout=out_dot)

  outpdf = outdot.replace(".dot", ".pdf")
  with open(outpdf, 'w') as opdf:
    subprocess.check_call(["dot", "-T", "pdf", f"-Glabel={init_number}",
                           outdot], stdout=opdf)
  init_number+=2
  #prompts.append(prompt)

  
