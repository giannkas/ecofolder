
import argparse

# Dictionaries for storing Petri net components
PL = {}
TR = {}
TR_presets = {}
TR_postsets = {}
PL_presets = {}
PL_postsets = {}

def pnet2rr():
  """
  pnet2rr converts a Petri net in the ll_net format to its corresponding rule system version.
  
  Usage: python3 pnet2rr.py input_file.ll_net 

  The input is a Petri net (in ll_net format), and the output is printed in the corresponding rule system format.

  Example:
    python3 pnet2rr.py examples/models/lambdaswitch/lambdaswitch.ll > examples/models/lambdaswitch/lambdaswitch.rr
  """

   # Argument parser for handling future expansions
  parser = argparse.ArgumentParser(description="Convert a Petri net (in ll_net format) to a rule system.")
  parser.add_argument("input_file", help="Path to the input .ll_net file (Petri net in ll_net format)")

  # Parse arguments
  args = parser.parse_args()

  plcount = 0
  trcount = 0

  # Reading from stdin (ll_net format)
  # Reading from the input file (ll_net format)
  with open(args.input_file, 'r') as f:
    for line in f:
      line = line.strip()

      if "PL" in line:  # Place section
        plcount += 1
      elif '"' in line and plcount > 0 and trcount == 0:  # Place line
        lsplit = line.split('"')
        plid = plcount if len(lsplit[0]) < 1 else int(lsplit[0])
        plname = lsplit[1]
        PL[plid] = plname
        plcount += 1
      elif "TR" in line:  # Transition section
        trcount += 1
      elif '"' in line and trcount > 0:  # Transition line
        lsplit = line.split('"')
        trid = trcount if len(lsplit[0]) < 1 else int(lsplit[0])
        trname = lsplit[1]
        TR[trid] = trname
        trcount += 1
      elif "<" in line:  # Postset relations (TR -> PL)
        lsplit = line.split('<')
        tr_id, pl_id = int(lsplit[0]), int(lsplit[1])

        if tr_id not in TR_postsets:
          TR_postsets[tr_id] = []
        TR_postsets[tr_id].append(PL[pl_id])

        if pl_id not in PL_presets:
          PL_presets[pl_id] = []
        PL_presets[pl_id].append(TR[tr_id])

      elif ">" in line:  # Preset relations (PL -> TR)
        lsplit = line.split('>')
        pl_id, tr_id = int(lsplit[0]), int(lsplit[1])

        if tr_id not in TR_presets:
          TR_presets[tr_id] = []
        TR_presets[tr_id].append(PL[pl_id])

        if pl_id not in PL_postsets:
          PL_postsets[pl_id] = []
        PL_postsets[pl_id].append(TR[tr_id])

  # Output the results in rule format
  for i in range(1, len(TR_presets) + 1):
    print(f"{i}: {', '.join(PL for PL in TR_presets[i])} >> {', '.join(PL for PL in TR_postsets[i])}")

if __name__ == "__main__":
  pnet2rr()
