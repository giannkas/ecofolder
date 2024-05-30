from sys import stdin

PL = {}
TR = {}
TR_presets = {}
TR_postsets = {}
PL_presets = {}
PL_postsets = {}

def main():
  line = stdin.readline()
  while len(line)!=0:
    if "M1" in line or "M0" in line:
      lsplit = line.split('"')
      plid = int(lsplit[0])
      plname = lsplit[1]
      PL[plid] = plname
    elif "@" in line:
      lsplit = line.split('"')
      trid = int(lsplit[0])
      trname = lsplit[1]
      TR[trid] = trname
    elif "<" in line:
      lsplit = line.split('<')
      if int(lsplit[0]) in TR_postsets:
        TR_postsets[int(lsplit[0])].append(PL[int(lsplit[1])])
      else: TR_postsets[int(lsplit[0])] = [PL[int(lsplit[1])]]
      if int(lsplit[1]) in PL_presets:
        PL_presets[int(lsplit[1])].append(TR[int(lsplit[0])])
      else: PL_presets[int(lsplit[1])] = [TR[int(lsplit[0])]]
    elif ">" in line:
      lsplit = line.split('>')
      if int(lsplit[1]) in TR_presets:
        TR_presets[int(lsplit[1])].append(PL[int(lsplit[0])])
      else: TR_presets[int(lsplit[1])] = [PL[int(lsplit[0])]]
      if int(lsplit[0]) in PL_postsets:
        PL_postsets[int(lsplit[0])].append(TR[int(lsplit[1])])
      else: PL_postsets[int(lsplit[0])] = [TR[int(lsplit[1])]]
    line = stdin.readline()

  # print(PL)
main()
for i in range(1,42):
  print(i, ": ", TR_presets[i], ">>", TR_postsets[i])