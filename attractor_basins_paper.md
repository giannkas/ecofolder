# Attractor basins in concurrent systems

This file is to guide the reproducibility of results informed in _Attractor Basins in Concurrent Systems_. After you have installed `ecofolder` and its prerequisites, from the parent folder (`ecofolder`), you can continue as follows:

## Lambdaswitch example

`./doomed --model examples/models/lambdaswitch/lambdaswitch --badmarkings examples/models/lambdaswitch/lambdaswitch.bad`

It outputs:

```bash
Prefix has 126 events, including cut-offs
Unchallenged events {'(pi0,e15)', '(pi0,e112)'}
prefix_d DiGraph with 126 nodes and 197 edges
maxconfs: 5it [00:00, 73.09it/s]
00m00s [MINDOO 1]        ['R12', 'R5', 'R22', 'R40'] ['(pi0,e2)', '(pi0,e8)', '(pi0,e14)', '(pi0,e120)']
Marking:  ('cro_2', 'cII_0', 'cI_0', 'N_0')
   free checks: 2
00m00s [MINDOO 2]        ['R12', 'R22'] ['(pi0,e2)', '(pi0,e7)']
Marking:  ('cro_2', 'cII_0', 'cI_0', 'N_0')
   free checks: 4
00m00s [MINDOO 3]        ['R30', 'R13', 'R23', 'R28'] ['(pi0,e1)', '(pi0,e5)', '(pi0,e12)', '(pi0,e115)']
Marking:  ('cro_2', 'cII_0', 'cI_0', 'N_0')
   free checks: 6
00m00s [MINDOO 4]        ['R12', 'R5', 'R22', 'R8', 'R40', 'R16'] ['(pi0,e2)', '(pi0,e8)', '(pi0,e14)', '(pi0,e22)', '(pi0,e24)', '(pi0,e97)']
Marking:  ('cro_2', 'cII_0', 'cI_0', 'N_0')
   free checks: 11
00m00s [MINDOO 5]        ['R12', 'R5', 'R22', 'R8', 'R1', 'R41', 'R16'] ['(pi0,e2)', '(pi0,e8)', '(pi0,e14)', '(pi0,e22)', '(pi0,e30)', '(pi0,e31)', '(pi0,e81)']
Marking:  ('cro_3', 'cII_0', 'cI_0', 'N_0')
   free checks: 13
total free checks: 13
```

## Mammalian cell cycle example

`./doomed --model examples/models/mammalian10/mammalian10 --badmarkings examples/models/mammalian10/mammalian10.bad`

It outputs:

```bash
Prefix has 176 events, including cut-offs
Unchallenged events {'(pi0,e69)', '(pi0,e144)', '(pi0,e118)', '(pi0,e103)', '(pi0,e20)', '(pi0,e26)', '(pi0,e128)', '(pi0,e172)', '(pi0,e68)', '(pi0,e41)', '(pi0,e97)', '(pi0,e14)', '(pi0,e62)', '(pi0,e99)'}
prefix_d DiGraph with 176 nodes and 344 edges
maxconfs: 7it [00:00, 84.69it/s]
EMPTY MINDOO
total free checks: 24
```

## Cell death receptor

`./doomed --model examples/models/celldeath/celldeath --badmarkings examples/models/celldeath/celldeath.bad`

It outputs:

```bash
Prefix has 791 events, including cut-offs
Unchallenged events {'(pi0,e717)', '(pi0,e677)', '(pi0,e676)'}
prefix_d DiGraph with 791 nodes and 1950 edges
maxconfs: 16it [00:00, 19.49it/s]
00m17s [MINDOO 1]        ['24', '12', '28', '11', '10'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e12)', '(pi0,e20)', '(pi0,e779)']
Marking:  ('ATP=1', 'C3=1', 'C8=1', 'FAS=0', 'MOMP=1', 'MPT=0', 'NFkB=0', 'RIP1=1', 'ROS=0', 'TNF=1', 'cIAP=0')
   free checks: 21
01m00s [MINDOO 2]        ['24', '12', '28', '26', '16', '31', '17', '11', '10'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e12)', '(pi0,e25)', '(pi0,e39)', '(pi0,e62)', '(pi0,e83)', '(pi0,e103)', '(pi0,e123)']
Marking:  ('ATP=1', 'C3=1', 'C8=0', 'FAS=0', 'MOMP=1', 'MPT=0', 'NFkB=0', 'RIP1=0', 'ROS=0', 'TNF=1', 'cIAP=0')
   free checks: 53
01m09s [MINDOO 3]        ['24', '12', '28', '26', '16', '31', '17', '11', '14'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e12)', '(pi0,e25)', '(pi0,e39)', '(pi0,e62)', '(pi0,e83)', '(pi0,e103)', '(pi0,e657)']
Marking:  ('ATP=1', 'C3=1', 'C8=1', 'FAS=0', 'MOMP=1', 'MPT=0', 'NFkB=0', 'RIP1=0', 'ROS=0', 'TNF=1', 'cIAP=1')
   free checks: 56
01m10s [MINDOO 4]        ['24', '12', '28', '16'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e12)', '(pi0,e787)']
Marking:  ('ATP=1', 'C3=0', 'C8=1', 'FAS=0', 'MOMP=1', 'MPT=0', 'NFkB=0', 'RIP1=0', 'ROS=0', 'TNF=1', 'cIAP=1')
   free checks: 58
01m47s [MINDOO 5]        ['24', '12', '20', '16'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e10)', '(pi0,e21)']
Marking:  ('ATP=1', 'C3=0', 'C8=1', 'FAS=0', 'MOMP=0', 'MPT=0', 'NFkB=0', 'RIP1=0', 'ROS=1', 'TNF=1', 'cIAP=1')
   free checks: 70
02m26s [MINDOO 6]        ['24', '12', '20', '6', '26', '27', '29', '10', '18'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e10)', '(pi0,e14)', '(pi0,e29)', '(pi0,e30)', '(pi0,e54)', '(pi0,e69)', '(pi0,e114)']
Marking:  ('ATP=1', 'C3=0', 'C8=1', 'FAS=0', 'MOMP=1', 'MPT=1', 'NFkB=0', 'RIP1=1', 'ROS=0', 'TNF=1', 'cIAP=0')
   free checks: 83
02m42s [MINDOO 7]        ['24', '12', '20', '6', '26', '27', '16', '31', '29', '17', '24', '12', '11', '10'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e10)', '(pi0,e14)', '(pi0,e29)', '(pi0,e30)', '(pi0,e51)', '(pi0,e75)', '(pi0,e97)', '(pi0,e117)', '(pi0,e148)', '(pi0,e185)', '(pi0,e235)', '(pi0,e355)']
Marking:  ('ATP=1', 'C3=1', 'C8=1', 'FAS=0', 'MOMP=1', 'MPT=1', 'NFkB=0', 'RIP1=1', 'ROS=0', 'TNF=1', 'cIAP=0')
   free checks: 93
02m55s [MINDOO 8]        ['24', '12', '20', '6', '26', '27', '16', '31', '29', '17', '11', '23', '1', '14'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e10)', '(pi0,e14)', '(pi0,e29)', '(pi0,e30)', '(pi0,e51)', '(pi0,e75)', '(pi0,e97)', '(pi0,e117)', '(pi0,e184)', '(pi0,e218)', '(pi0,e225)', '(pi0,e383)']
Marking:  ('ATP=0', 'C3=1', 'C8=1', 'FAS=0', 'MOMP=1', 'MPT=0', 'NFkB=0', 'RIP1=0', 'ROS=0', 'TNF=1', 'cIAP=1')
   free checks: 95
02m58s [MINDOO 9]        ['24', '12', '28', '10'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e12)', '(pi0,e19)']
Marking:  ('ATP=1', 'C3=0', 'C8=1', 'FAS=0', 'MOMP=1', 'MPT=0', 'NFkB=0', 'RIP1=1', 'ROS=0', 'TNF=1', 'cIAP=0')
   free checks: 97
03m54s [MINDOO 10]       ['12'] ['(pi0,e1)']
Marking:  ('ATP=1', 'C3=0', 'C8=1', 'FAS=0', 'MOMP=0', 'MPT=0', 'NFkB=0', 'RIP1=0', 'ROS=0', 'TNF=1', 'cIAP=1')
   free checks: 119
04m02s [MINDOO 11]       ['24', '12', '20', '6', '26', '16', '31', '29', '17', '24', '12', '16'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e10)', '(pi0,e14)', '(pi0,e29)', '(pi0,e51)', '(pi0,e75)', '(pi0,e97)', '(pi0,e117)', '(pi0,e148)', '(pi0,e185)', '(pi0,e473)']
Marking:  ('ATP=1', 'C3=0', 'C8=1', 'FAS=0', 'MOMP=0', 'MPT=1', 'NFkB=0', 'RIP1=0', 'ROS=0', 'TNF=1', 'cIAP=1')
   free checks: 121
04m58s [MINDOO 12]       ['24', '12', '20', '6', '26', '27', '16', '31', '29', '17', '11', '10'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e10)', '(pi0,e14)', '(pi0,e29)', '(pi0,e30)', '(pi0,e51)', '(pi0,e75)', '(pi0,e97)', '(pi0,e117)', '(pi0,e184)', '(pi0,e217)']
Marking:  ('ATP=1', 'C3=1', 'C8=0', 'FAS=0', 'MOMP=1', 'MPT=1', 'NFkB=0', 'RIP1=0', 'ROS=0', 'TNF=1', 'cIAP=0')
   free checks: 128
05m18s [MINDOO 13]       ['24', '12', '20', '6', '26', '16', '29', '17'] ['(pi0,e2)', '(pi0,e3)', '(pi0,e10)', '(pi0,e14)', '(pi0,e29)', '(pi0,e51)', '(pi0,e54)', '(pi0,e95)']
Marking:  ('ATP=1', 'C3=0', 'C8=1', 'FAS=0', 'MOMP=0', 'MPT=1', 'NFkB=0', 'RIP1=0', 'ROS=0', 'TNF=1', 'cIAP=1')
   free checks: 135
08m46s [MINDOO 14]       ['24', '20', '6', '26'] ['(pi0,e2)', '(pi0,e5)', '(pi0,e7)', '(pi0,e16)']
Marking:  ('ATP=1', 'C3=0', 'C8=0', 'FAS=0', 'MOMP=0', 'MPT=1', 'NFkB=1', 'RIP1=1', 'ROS=1', 'TNF=1', 'cIAP=1')
   free checks: 146
total free checks: 146
```

## Budding yeast cell cycle

`./doomed --model examples/models/budding_yeast/budding_yeast --badmarkings examples/models/budding_yeast/budding_yeast.bad`

It outputs:

```bash
Prefix has 1413 events, including cut-offs
Unchallenged events {'(pi0,e1411)', '(pi0,e1260)', '(pi0,e1287)', '(pi0,e883)'}
prefix_d DiGraph with 1412 nodes and 3245 edges
maxconfs: 30it [00:03,  9.73it/s]
00m19s [MINDOO 1]        [] []
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=1', 'ACE2=0', 'SWI5=0', 'CLN3=0')
   free checks: 27
00m21s [MINDOO 2]        ['13', '28', '14', '29', '1', '23'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e1405)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 34
00m25s [MINDOO 3]        ['13', '28', '14', '19', '6', '18', '5', '8', '27', '16', '17', '15', '20', '6', '8', '29', '1', '17', '31', '23'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e10)', '(pi0,e13)', '(pi0,e20)', '(pi0,e30)', '(pi0,e40)', '(pi0,e55)', '(pi0,e82)', '(pi0,e119)', '(pi0,e158)', '(pi0,e198)', '(pi0,e239)', '(pi0,e279)', '(pi0,e316)', '(pi0,e333)', '(pi0,e340)', '(pi0,e360)', '(pi0,e688)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 40
00m27s [MINDOO 4]        ['13', '28', '14', '29', '1', '19', '18', '2', '23', '16', '9'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e22)', '(pi0,e27)', '(pi0,e35)', '(pi0,e63)', '(pi0,e98)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=1', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 42
00m31s [MINDOO 5]        ['13', '28', '14', '29', '1', '19', '4', '18', '2', '23', '3', '16', '9', '29'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e17)', '(pi0,e22)', '(pi0,e27)', '(pi0,e35)', '(pi0,e59)', '(pi0,e63)', '(pi0,e167)', '(pi0,e207)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=1', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=0', 'CLN3=0')
   free checks: 47
00m31s [MINDOO 6]        ['13', '28', '14', '29', '18', '1', '23', '16'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e9)', '(pi0,e11)', '(pi0,e23)', '(pi0,e1392)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 49
00m37s [MINDOO 7]        ['13', '28', '14', '19', '6', '18', '2', '5', '8', '27', '16', '17', '15', '29', '1', '20', '4', '3', '6', '8', '17', '11', '31', '18', '23', '16', '29'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e10)', '(pi0,e13)', '(pi0,e20)', '(pi0,e26)', '(pi0,e36)', '(pi0,e49)', '(pi0,e73)', '(pi0,e110)', '(pi0,e151)', '(pi0,e190)', '(pi0,e191)', '(pi0,e266)', '(pi0,e278)', '(pi0,e305)', '(pi0,e350)', '(pi0,e369)', '(pi0,e385)', '(pi0,e400)', '(pi0,e415)', '(pi0,e431)', '(pi0,e444)', '(pi0,e452)', '(pi0,e467)', '(pi0,e615)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=0', 'CLN3=0')
   free checks: 58
00m41s [MINDOO 8]        ['13', '28', '14', '29', '1', '19', '23'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e24)']
Marking:  ('MBF=0', 'SBF=1', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 64
00m50s [MINDOO 9]        ['13', '28', '14', '19', '6', '18', '2', '5', '8', '27', '16', '17', '15', '29', '1', '20', '4', '3', '6', '8', '17', '31', '23', '29'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e10)', '(pi0,e13)', '(pi0,e20)', '(pi0,e26)', '(pi0,e36)', '(pi0,e49)', '(pi0,e73)', '(pi0,e110)', '(pi0,e151)', '(pi0,e190)', '(pi0,e191)', '(pi0,e266)', '(pi0,e278)', '(pi0,e305)', '(pi0,e350)', '(pi0,e369)', '(pi0,e385)', '(pi0,e400)', '(pi0,e418)', '(pi0,e569)', '(pi0,e615)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=1', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=0', 'CLN3=0')
   free checks: 71
00m54s [MINDOO 10]       ['13', '28', '14', '19', '6', '18', '5', '8', '27', '29', '1', '16'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e10)', '(pi0,e13)', '(pi0,e20)', '(pi0,e30)', '(pi0,e40)', '(pi0,e55)', '(pi0,e56)', '(pi0,e65)', '(pi0,e82)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=1', 'HCM1=0', 'YHP1=1', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 81
01m00s [MINDOO 11]       ['13', '28', '14', '19', '6', '18', '5', '8', '27', '29', '4', '16'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e10)', '(pi0,e13)', '(pi0,e20)', '(pi0,e30)', '(pi0,e40)', '(pi0,e55)', '(pi0,e56)', '(pi0,e78)', '(pi0,e82)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=1', 'HCM1=0', 'YHP1=1', 'SFF=0', 'ACE2=1', 'SWI5=0', 'CLN3=0')
   free checks: 95
01m04s [MINDOO 12]       ['13', '28', '14', '29', '1', '19', '4', '6', '18', '2', '23', '3', '16', '9', '29'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e17)', '(pi0,e21)', '(pi0,e32)', '(pi0,e38)', '(pi0,e47)', '(pi0,e89)', '(pi0,e97)', '(pi0,e205)', '(pi0,e252)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=1', 'YHP1=1', 'SFF=0', 'ACE2=0', 'SWI5=0', 'CLN3=0')
   free checks: 98
01m08s [MINDOO 13]       ['28', '29'] ['(pi0,e2)', '(pi0,e6)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 104
01m12s [MINDOO 14]       ['13', '28', '14', '29', '1', '19', '18', '23', '5', '2', '3', '16', '13', '15', '6', '8', '29', '1', '17', '31', '23'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e22)', '(pi0,e35)', '(pi0,e45)', '(pi0,e62)', '(pi0,e92)', '(pi0,e96)', '(pi0,e131)', '(pi0,e215)', '(pi0,e260)', '(pi0,e300)', '(pi0,e328)', '(pi0,e343)', '(pi0,e355)', '(pi0,e376)', '(pi0,e672)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=1', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 106
01m18s [MINDOO 15]       ['13', '28', '14', '19', '6', '18', '2', '5', '8', '27', '16', '17', '15', '29', '1', '20', '4', '3', '6', '8', '17', '11', '31', '23', '29'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e10)', '(pi0,e13)', '(pi0,e20)', '(pi0,e26)', '(pi0,e36)', '(pi0,e49)', '(pi0,e73)', '(pi0,e110)', '(pi0,e151)', '(pi0,e190)', '(pi0,e191)', '(pi0,e266)', '(pi0,e278)', '(pi0,e305)', '(pi0,e350)', '(pi0,e369)', '(pi0,e385)', '(pi0,e400)', '(pi0,e415)', '(pi0,e431)', '(pi0,e531)', '(pi0,e615)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=0', 'CLN3=0')
   free checks: 109
01m22s [MINDOO 16]       ['13', '28', '14', '29', '1', '19', '4', '18', '23', '5', '2', '16', '3', '11', '30'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e17)', '(pi0,e22)', '(pi0,e35)', '(pi0,e45)', '(pi0,e62)', '(pi0,e96)', '(pi0,e125)', '(pi0,e204)', '(pi0,e250)']
Marking:  ('MBF=0', 'SBF=1', 'YOX1=1', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=0', 'CLN3=0')
   free checks: 114
01m26s [MINDOO 17]       ['13', '28', '14', '29', '4', '19', '18', '2', '24', '16', '9'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e12)', '(pi0,e16)', '(pi0,e22)', '(pi0,e27)', '(pi0,e39)', '(pi0,e72)', '(pi0,e109)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=1', 'YHP1=0', 'SFF=0', 'ACE2=1', 'SWI5=0', 'CLN3=0')
   free checks: 116
01m28s [MINDOO 18]       ['13', '28', '14', '29', '4', '19', '24'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e12)', '(pi0,e16)', '(pi0,e29)']
Marking:  ('MBF=0', 'SBF=1', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=1', 'SWI5=0', 'CLN3=0')
   free checks: 118
01m29s [MINDOO 19]       ['13', '28', '14', '19', '6', '18', '5', '8', '27', '29', '16', '31', '14', '1', '23'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e10)', '(pi0,e13)', '(pi0,e20)', '(pi0,e30)', '(pi0,e40)', '(pi0,e55)', '(pi0,e56)', '(pi0,e82)', '(pi0,e120)', '(pi0,e157)', '(pi0,e223)', '(pi0,e1032)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=0', 'YHP1=1', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 120
01m33s [MINDOO 20]       ['13', '28', '14', '29', '4', '19', '18', '24', '5', '16'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e12)', '(pi0,e16)', '(pi0,e22)', '(pi0,e39)', '(pi0,e52)', '(pi0,e80)']
Marking:  ('MBF=0', 'SBF=1', 'YOX1=1', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=1', 'SWI5=0', 'CLN3=0')
   free checks: 126
01m39s [MINDOO 21]       ['13', '28', '14', '29', '1', '19', '6', '18', '5', '27', '16'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e21)', '(pi0,e32)', '(pi0,e42)', '(pi0,e58)', '(pi0,e86)']
Marking:  ('MBF=0', 'SBF=1', 'YOX1=1', 'HCM1=0', 'YHP1=1', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 137
01m40s [MINDOO 22]       ['13', '28', '14', '29', '18', '4', '24', '16'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e9)', '(pi0,e12)', '(pi0,e28)', '(pi0,e1384)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=1', 'SWI5=0', 'CLN3=0')
   free checks: 139
01m42s [MINDOO 23]       ['13', '28', '14', '29', '1', '19', '4', '6', '18', '2', '23', '3', '16', '11', '30'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e17)', '(pi0,e21)', '(pi0,e32)', '(pi0,e38)', '(pi0,e47)', '(pi0,e89)', '(pi0,e97)', '(pi0,e206)', '(pi0,e253)']
Marking:  ('MBF=0', 'SBF=1', 'YOX1=0', 'HCM1=0', 'YHP1=1', 'SFF=0', 'ACE2=0', 'SWI5=0', 'CLN3=0')
   free checks: 141
01m44s [MINDOO 24]       ['13', '28', '14', '29', '1', '19', '4', '18', '23', '5', '2', '16', '3', '10', '29'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e17)', '(pi0,e22)', '(pi0,e35)', '(pi0,e45)', '(pi0,e62)', '(pi0,e96)', '(pi0,e125)', '(pi0,e203)', '(pi0,e249)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=1', 'HCM1=1', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=0', 'CLN3=0')
   free checks: 143
01m45s [MINDOO 25]       ['13', '28', '14', '29', '1', '19', '18', '23', '5', '16'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e22)', '(pi0,e35)', '(pi0,e45)', '(pi0,e67)']
Marking:  ('MBF=0', 'SBF=1', 'YOX1=1', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 148
01m50s [MINDOO 26]       ['13', '29'] ['(pi0,e1)', '(pi0,e5)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=0', 'YHP1=0', 'SFF=0', 'ACE2=1', 'SWI5=0', 'CLN3=0')
   free checks: 157
01m57s [MINDOO 27]       ['13', '28', '14', '29', '1', '19', '18', '2', '23', '3', '16', '13', '9', '14', '29', '1', '23'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e11)', '(pi0,e16)', '(pi0,e22)', '(pi0,e27)', '(pi0,e35)', '(pi0,e43)', '(pi0,e63)', '(pi0,e93)', '(pi0,e134)', '(pi0,e217)', '(pi0,e218)', '(pi0,e282)', '(pi0,e904)']
Marking:  ('MBF=0', 'SBF=0', 'YOX1=0', 'HCM1=1', 'YHP1=0', 'SFF=0', 'ACE2=0', 'SWI5=1', 'CLN3=0')
   free checks: 162
01m59s [MINDOO 28]       ['13', '28', '14', '29', '4', '19', '6', '18', '5', '27', '16'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)', '(pi0,e8)', '(pi0,e12)', '(pi0,e16)', '(pi0,e21)', '(pi0,e32)', '(pi0,e42)', '(pi0,e58)', '(pi0,e86)']
Marking:  ('MBF=0', 'SBF=1', 'YOX1=1', 'HCM1=0', 'YHP1=1', 'SFF=0', 'ACE2=1', 'SWI5=0', 'CLN3=0')
   free checks: 165
total free checks: 165
```
