PEP
PTNet
FORMAT_N
PL
1"Bclaf1=0"M1
2"Bclaf1=1"M0
3"CDK46CycD=0"M0
4"CDK46CycD=1"M1
5"CIPKIP=0"M0
6"CIPKIP=1"M1
7"Cebpa=0"M1
8"Cebpa=1"M0
9"Egr1=0"M0
10"Egr1=1"M1
11"Fli1=0"M0
12"Fli1=1"M1
13"Gata1=0"M1
14"Gata1=1"M0
15"Gata2=0"M0
16"Gata2=1"M1
17"Ikzf1=0"M1
18"Ikzf1=1"M0
19"Junb=0"M0
20"Junb=1"M1
21"Klf1=0"M1
22"Klf1=1"M0
23"Myc=0"M0
24"Myc=1"M1
25"Spi1=0"M1
26"Spi1=1"M0
27"Tal1=0"M0
28"Tal1=1"M1
29"Zfpm1=0"M1
30"Zfpm1=1"M0
TR
1"Bclaf1=0 -> Bclaf1=1 when Myc=1"
2"Bclaf1=1 -> Bclaf1=0 when Myc=0"
3"CDK46CycD=0 -> CDK46CycD=1 when Bclaf1=1"
4"CDK46CycD=0 -> CDK46CycD=1 when Myc=1"
5"CDK46CycD=1 -> CDK46CycD=0 when Bclaf1=0 and Myc=0"
6"CIPKIP=0 -> CIPKIP=1 when Junb=1"
7"CIPKIP=1 -> CIPKIP=0 when Junb=0"
8"Cebpa=0 -> Cebpa=1 when Gata2=1 and Ikzf1=0"
9"Cebpa=0 -> Cebpa=1 when Ikzf1=0 and Spi1=1"
10"Cebpa=1 -> Cebpa=0 when Ikzf1=1"
11"Cebpa=1 -> Cebpa=0 when Gata2=0 and Spi1=0"
12"Egr1=0 -> Egr1=1 when Gata2=1 and Junb=1"
13"Egr1=1 -> Egr1=0 when Gata2=0"
14"Egr1=1 -> Egr1=0 when Junb=0"
15"Fli1=0 -> Fli1=1 when Junb=1"
16"Fli1=0 -> Fli1=1 when Gata1=1 and Klf1=0"
17"Fli1=1 -> Fli1=0 when Gata1=0 and Junb=0"
18"Fli1=1 -> Fli1=0 when Junb=0 and Klf1=1"
19"Gata1=0 -> Gata1=1 when Fli1=1"
20"Gata1=0 -> Gata1=1 when Gata2=1 and Spi1=0"
21"Gata1=1 -> Gata1=0 when Fli1=0 and Gata2=0 and Ikzf1=1"
22"Gata1=1 -> Gata1=0 when Fli1=0 and Spi1=1"
23"Gata2=0 -> Gata2=1 when Egr1=1 and Gata1=0 and Spi1=0 and Zfpm1=0"
24"Gata2=1 -> Gata2=0 when Gata1=1"
25"Gata2=1 -> Gata2=0 when Zfpm1=1"
26"Ikzf1=0 -> Ikzf1=1 when Gata2=1"
27"Ikzf1=1 -> Ikzf1=0 when Gata2=0"
28"Junb=0 -> Junb=1 when Egr1=1"
29"Junb=0 -> Junb=1 when Myc=1"
30"Junb=1 -> Junb=0 when Egr1=0 and Myc=0"
31"Klf1=0 -> Klf1=1 when Fli1=0 and Gata1=1"
32"Klf1=1 -> Klf1=0 when Fli1=1"
33"Klf1=1 -> Klf1=0 when Gata1=0"
34"Myc=0 -> Myc=1 when Bclaf1=1 and Cebpa=1"
35"Myc=1 -> Myc=0 when Bclaf1=0"
36"Myc=1 -> Myc=0 when Cebpa=0"
37"Spi1=0 -> Spi1=1 when Cebpa=1 and Gata1=0 and Gata2=0"
38"Spi1=1 -> Spi1=0 when Gata1=1"
39"Tal1=0 -> Tal1=1 when Gata1=1 and Spi1=0"
40"Tal1=1 -> Tal1=0 when Gata1=0"
41"Tal1=1 -> Tal1=0 when Spi1=1"
42"Zfpm1=0 -> Zfpm1=1 when Gata1=1"
43"Zfpm1=1 -> Zfpm1=0 when Gata1=0"
TP
1<2
1<24
2<1
2<23
3<2
3<4
4<4
4<24
5<1
5<3
5<23
6<6
6<20
7<5
7<19
8<8
8<16
8<17
9<8
9<17
9<26
10<7
10<18
11<7
11<15
11<25
12<10
12<16
12<20
13<9
13<15
14<9
14<19
15<12
15<20
16<12
16<14
16<21
17<11
17<13
17<19
18<11
18<19
18<22
19<12
19<14
20<14
20<16
20<25
21<11
21<13
21<15
21<18
22<11
22<13
22<26
23<10
23<13
23<16
23<25
23<29
24<14
24<15
25<15
25<30
26<16
26<18
27<15
27<17
28<10
28<20
29<20
29<24
30<9
30<19
30<23
31<11
31<14
31<22
32<12
32<21
33<13
33<21
34<2
34<8
34<24
35<1
35<23
36<7
36<23
37<8
37<13
37<15
37<26
38<14
38<25
39<14
39<25
39<28
40<13
40<27
41<26
41<27
42<14
42<30
43<13
43<29
PT
1>1
1>5
1>35
2>2
2>3
2>34
3>3
3>4
4>5
5>6
6>7
7>8
7>9
7>36
8>10
8>11
8>34
8>37
9>12
9>30
10>13
10>14
10>23
10>28
11>15
11>16
11>21
11>22
11>31
12>17
12>18
12>19
12>32
13>17
13>19
13>20
13>23
13>33
13>37
13>40
13>43
14>16
14>21
14>22
14>24
14>31
14>38
14>39
14>42
15>11
15>13
15>21
15>23
15>27
15>37
16>8
16>12
16>20
16>24
16>25
16>26
17>8
17>9
17>26
18>10
18>21
18>27
19>7
19>14
19>17
19>18
19>28
19>29
20>6
20>12
20>15
20>30
21>16
21>31
22>18
22>32
22>33
23>2
23>5
23>30
23>34
24>1
24>4
24>29
24>35
24>36
25>11
25>20
25>23
25>37
25>39
26>9
26>22
26>38
26>41
27>39
28>40
28>41
29>23
29>42
30>25
30>43
