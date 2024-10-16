import sys
import subprocess
import os
import re
import argparse

# ./color_in_dot -ru "R1,R2,R3,R4,R5,R6,R7,R8,R9,R10,R11;R12,R13;R14,R15,R16;R17,R18,R19,R20,R21,R22" -co "#1a9850;#d73027;#fee08b;#8c510a" -cocs "none" < examples/oursin_tasmanie/tasmanian_perturb_final_pr_urc2alg_unf.dot > examples/oursin_tasmanie/tasmanian_perturb_final_pr_urc2alg_unf_colored.dot

# ./prompts_color_firing_sequence examples/oursin_tasmanie/minconfs-to-marking_tasmanian_perturb_final_pr_urc2alg.evev examples/oursin_tasmanie/tasmanian_perturb_final_pr_urc2alg_unf_colored.dot "#1a9850;#d73027;#fee08b;#8c510a;gray60" "11;13;16;17" examples/oursin_tasmanie/mrk_instances/urc2alg+/ 5

# python3 prompts_labels_colored_firing_sequence.py examples/oursin_tasmanie/minconfs-to-marking_tasmanian_perturb_final_pr_urc2alg.evev examples/oursin_tasmanie/mrk_instances/urc2alg+/tasmanian_perturb_final_pr_urc2alg_unf_colored_ins1.dot examples/oursin_tasmanie/tasmanian_perturb_final.rr examples/oursin_tasmanie/mrk_instances/urc2alg+/ 6

def process_evev_file(evev_path, dot_file, rr_path, outdir, init_number):
  """
  Processes an 'evev' file, modifies .dot files, and converts them to labeled PDF files.
  
  Args:
      evev_path (str): Path to the 'evev' file to be read.
      dot_file (str): Path to the initial .dot file to modify and label.
      rr_path (str): Path to the .rr file used for rule-based labeling.
      outdir (str): Output directory to save the labeled .dot and PDF files.
      init_number (int): Initial number for the graph label.
  """
  
  # Read 'evev' file
  with open(evev_path, "r") as file:
    evev_lines = file.readlines()

  for line_number, line in enumerate(evev_lines, start=1):
    # Modify the dot file name to reflect current iteration
    ins_len = len(re.search(r'ins[0-9]+', dot_file).group())
    ins_pos = dot_file.rfind("ins")
    dot_name = dot_file[:ins_pos] + f"ins{line_number}" + dot_file[ins_pos+ins_len:]
    
    # Prepare output file names
    outfile = os.path.basename(dot_name.replace(".dot", ""))
    outdot = f"{outdir}/{outfile}_labels.dot"

    # Call Python script to generate labeled .dot file
    with open(outdot, 'w') as out_dot:
      subprocess.check_call(["python3", "ids-labels_rules.py", rr_path, dot_name], stdout=out_dot)

    # Generate the PDF from the labeled .dot file
    outpdf = outdot.replace(".dot", ".pdf")
    with open(outpdf, 'w') as opdf:
      subprocess.check_call(["dot", "-T", "pdf", f"-Glabel={init_number}", outdot], stdout=opdf)

    # Increment init_number for the next graph
    init_number += 2

if __name__ == "__main__":
  # Set up argument parser
  parser = argparse.ArgumentParser(description="Process an 'evev' file, modify .dot files, and convert them to labeled PDF files.")
  
  # Add arguments
  parser.add_argument("evev_path", help="Path to the 'evev' file to be read.")
  parser.add_argument("dot_file", help="Path to the initial .dot file to modify and label.")
  parser.add_argument("rr_path", help="Path to the .rr file used for rule-based labeling.")
  parser.add_argument("outdir", help="Output directory to save the labeled .dot and PDF files.")
  parser.add_argument("init_number", type=int, help="Initial number for the graph label.")
  
  # Parse the arguments
  args = parser.parse_args()
  
  # Call the function with parsed arguments
  process_evev_file(args.evev_path, args.dot_file, args.rr_path, args.outdir, args.init_number)