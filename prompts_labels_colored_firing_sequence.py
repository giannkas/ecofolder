import sys
import subprocess
import os
import re

evev_path = sys.argv[1]
dot_file = sys.argv[2]
rr_path = sys.argv[3]
outdir = sys.argv[4]
init_number = int(sys.argv[5])

# Read 'evev' file
with open(evev_path, "r") as file:
  evev_lines = file.readlines()

for line_number, line in enumerate(evev_lines, start=1):

  ins_len = len(re.search(r'ins[0-9]+',dot_file).group())
  ins_pos = dot_file.rfind("ins")
  dot_name = dot_file[:ins_pos] + f"ins{line_number}" + dot_file[ins_pos+ins_len:]
 
  outfile = os.path.basename(dot_name.replace(".dot",""))
  outdot = f"{outdir}{outfile}_labels.dot"

  with open(outdot, 'w') as out_dot:
    subprocess.check_call(["python3", "ids-labels_rules.py", rr_path, dot_name],
                          stdout=out_dot)

  outpdf = outdot.replace(".dot", ".pdf")
  with open(outpdf, 'w') as opdf:
    subprocess.check_call(["dot", "-T", "pdf", f"-Glabel={init_number}",
                           outdot], stdout=opdf)
  
  init_number+=2