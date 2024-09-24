import sys
import subprocess
import os
import re

# ./color_in_dot -ru "R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11;R12,R13;R14,R15,R16;R17,R18,R19,R20,R21,R22" -co "#1a9850;#d73027;#fee08b;#8c510a" -cocs "none" < examples/oursin_tasmanie/tasmanian_perturb_final_pr_urc2alg_unf.dot > examples/oursin_tasmanie/tasmanian_perturb_final_pr_urc2alg_unf_colored.dot

# ./prompts_color_firing_sequence examples/oursin_tasmanie/minconfs-to-marking_tasmanian_perturb_final_pr_urc2alg.evev examples/oursin_tasmanie/tasmanian_perturb_final_pr_urc2alg_unf_colored.dot "#1a9850;#d73027;#fee08b;#8c510a;gray60" "11;13;16;17" examples/oursin_tasmanie/mrk_instances/urc2alg+/ 5

# python3 prompts_labels_colored_firing_sequence.py examples/oursin_tasmanie/minconfs-to-marking_tasmanian_perturb_final_pr_urc2alg.evev examples/oursin_tasmanie/mrk_instances/urc2alg+/tasmanian_perturb_final_pr_urc2alg_unf_colored_ins1.dot examples/oursin_tasmanie/tasmanian_perturb_final.rr examples/oursin_tasmanie/mrk_instances/urc2alg+/ 6

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