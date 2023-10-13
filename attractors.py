#!/usr/bin/env python

import shutil
import subprocess
import sys
import os

DEVNULL = open(os.devnull, 'wb')

class Model:
  def __init__(self, filename):
    if os.path.isfile(filename + ".ll_net"):
      self.filename = filename + ".ll_net"
    else:
      self.filename = filename + ".ll"
    self.suffix = self.filename.split('.')[-1]
    self.read(open(self.filename))

  def read(self, f):
    self.header = ''
    self.PL = {}
    self.TR = []
    self.RT = []
    self.RD = []
    self.TP = []
    self.PT = []
    self.RS = []
    self.PL_ = {}
    self.TR_ = []
    self.RT_ = []
    self.RD_ = []
    self.TP_ = []
    self.PT_ = []
    self.RS_ = []
    state = "header"
    max_pid = 0
    max_tid = 0
    pid, tid = 0, 0
    for l in f:
      r = l.strip()
      if r in ["PL", "TR", "RT", "RD", "TP", "PT", "RS"]:
        state = r
      elif state == "header":
        self.header += l
      elif state == "PL":
        parts = r.split('"')
        if len(parts[0]) < 1: pid += 1
        else: pid = int(parts[0])
        name = parts[1]
        m0 = "M1" in r
        self.PL[name] = {"id": pid, "m0": m0}
        max_pid = max(max_pid, pid)
      elif state == "TR":
        parts = r.split('"')
        if len(parts[0]) < 1: tid += 1
        else: tid = int(parts[0])
        max_tid = max(max_tid, tid)
        self.TR.append(r)
      elif state == "RT":
        self.RT.append(r)
      elif state == "RD":
        self.RD.append(r)
      elif state == "TP":
        self.TP.append(r)
      elif state == "PT":
        self.PT.append(r)
      elif state == "RS":
        self.RS.append(r)
    self.max_pid = max_pid
    self.max_tid = max_tid


  def write(self, f):
    f.write(self.header)
    PL = ["%d\"%s\"M%d" % (v["id"], k, 1 if v["m0"] else 0)
            for k, v in self.PL.items()] + self.PL_
    f.write("PL\n%s\n" % ("\n".join(PL)))
    f.write("TR\n%s\n" % ("\n".join(self.TR + self.TR_)))
    f.write("RT\n%s\n" % ("\n".join(self.RT + self.RT_)))
    f.write("RD\n%s\n" % ("\n".join(self.RD + self.RD_)))
    f.write("TP\n%s\n" % ("\n".join(self.TP + self.TP_)))
    f.write("PT\n%s\n" % ("\n".join(self.PT + self.PT_)))
    f.write("RS\n%s\n" % ("\n".join(self.RS + self.RS_)))

  def set_marking(self, m0):
    print(m0)
    for k in self.PL:
      self.PL[k]["m0"] = k in m0

  def marking_to_transition(self, m):
    tid = self.max_tid + 1
    self.max_tid = tid
    
    self.TR_.append("%d\"%s\"" % (tid, " ".join(m)))
    for p in m:
      pid = self.PL[p]["id"]
      self.PT_.append("%d>%d" % (pid, tid))
    return tid

  def attractors(self):
    # 1. compute first complete prefix
    mci0 = self.filename.replace("."+self.suffix, ".mci")
    subprocess.call(["ecofolder", "-data", self.filename, "-m", mci0])

    # 2. fetch final markings
    out = subprocess.check_output(["allfinals.pl", "-f", mci0], stderr=DEVNULL)
    markings = [tuple(sorted(l.split())) for l in out.strip().split(b'\n')]
    l_ret = len(markings)
    markings = list(set(markings))
    sys.stderr.write("### filtering %d (%d) final markings ...\n" % (len(markings), l_ret))
    for i in range(len(markings)):
      markings[i] = tuple(elem.decode('utf-8') for elem in markings[i])
    
    attractors = []
    for i, m in enumerate(markings):
      if not self.reach_any(m, attractors+markings[i+1:], len(attractors)):
        sys.stderr.write("+")
        sys.stderr.flush()
        attractors.append(m)
      else:
        sys.stderr.write(".")
        sys.stderr.flush()
    sys.stderr.write("\n")
    sys.stderr.flush()
    return attractors

  def reach_any(self, m0, markings, num=0):
    self.set_marking(m0)
    t = []
    self.PL_ = []
    self.TR_ = []
    self.RT_ = []
    self.RD_ = []
    self.TP_ = []
    self.PT_ = []
    self.RS_ = []
    for m in markings:
      t.append(self.marking_to_transition(m))
    rid = self.max_pid + 1
    tid = self.max_tid + 1
    self.PL_.append("%d\"reach_any\"" % rid)
    self.TR_.append("%d\"reached_any\"" % tid)
    self.PT_.append("%d>%d" % (rid, tid))
    for tid in t:
      self.TP_.append("%d<%d" % (tid, rid))

    name = "tmp."+self.suffix
    f = open(name, "w")
    self.write(f)
    f.close()

    subprocess.call(["ecofolder", "-data", "-T", "reached_any", name, "-m", "tmp.mci"])
    ret = subprocess.call(["mci2dot", "-reach", "reached_any", "tmp.mci"])

    reach = ret == 0

    if not reach:
      rname = self.filename.replace("."+self.suffix, "-a%d" % num)
      shutil.copyfile("tmp.mci", "%s.mci"%rname)
      shutil.copyfile("tmp."+self.suffix, "%s."%rname+self.suffix)

    os.unlink(name)
    os.unlink("tmp.mci")

    return reach


if __name__ == "__main__":
  m = Model(sys.argv[1])
  for a in sorted(m.attractors()):
    print(" ".join(a))

