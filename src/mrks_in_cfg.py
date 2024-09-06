import os
import sys
import time
import subprocess
from doomed import asp_of_mci,cfg_from_atoms,script_path,Model

from tqdm import tqdm

import clingo


def repeated_markings_in_conf():
  repeated_markings_in_conf.__doc__ = f"""
    {repeated_markings_in_conf.__name__} outputs the configurations leading to a marking and a '+' sign for those events that produce redundant markings.\n
    Usage: python3 {repeated_markings_in_conf.__name__} [files] [options]\n

    Files:
      -prx --prefix <mcifile>     prefix in .mci format from where the markings will be computed.
      -cfgs --configurations <evevfile>     list of configurations in the .evev file.\n
          
    Files are mandatory to compute the markings.\n
  """

  model_unf = ""
  evev_path = ""

  params = len(sys.argv)

  for i in range(params):
    if sys.argv[i] == "-prx" or sys.argv[i] == "--prefix":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
          raise ValueError(repeated_markings_in_conf.__doc__)
      model_unf = sys.argv[i]
    elif sys.argv[i] == "-cfgs" or sys.argv[i] == "--configurations":
      i += 1
      if (i == params or '-' == sys.argv[i][0]):
          raise ValueError(repeated_markings_in_conf.__doc__)
      evev_path = sys.argv[i]

  # Read 'evev' file
  with open(evev_path, "r+") as file:
    evev_lines = file.readlines()

    newevev = [[] for _ in range(len(evev_lines))]

    for conf_number, conf in enumerate(evev_lines):
      known = []
      conf = conf.split(' ')[:-1]
      for ev_number, _ in enumerate(conf,start=1):
        args_mrk = [script_path("mci2dot"), "-cf", ",".join(e for e in conf[:ev_number]), model_unf]
        mrk = subprocess.run(args_mrk,capture_output=True,text=True).stdout.strip()
        ev_pos = ev_number-1
        set_mrk = {pl for pl in mrk.split(',')}
        if set_mrk in known:
          newevev[conf_number].append(conf[ev_pos]+'+')
        else:
          newevev[conf_number].append(conf[ev_pos])
          known.append(set_mrk)

    file.seek(0)
    for conf in newevev:
      file.write(" ".join(e for e in conf))
      file.write(" 0\n")

if __name__ == "__main__":
  repeated_markings_in_conf()