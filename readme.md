# Ecofolder - A program to unfold Petri nets with resets

## General info

Ecofolder is a software for unfolding Petri nets with reset arcs, its architecture is quite similar to [**Mole - A Petri Net Unfolder**](http://www.lsv.fr/~schwoon/tools/mole/). It provides a data structure for a net's unfolding and a way to visualize it in a .pdf file using the [**DOT language**](https://graphviz.org/doc/info/lang.html). Ecofolder aims to deliver means for analyzing ecosystem dynamics in nature, but its usage is not restricted to those particular systems, i.e. one can also work with for example, gene regulatory networks..

## Installation

Typing `make` in the directory with the sources should do the trick. You can place the resulting executable in whichever directory you want.

You can run and see one of the examples as follows:

```console
./ecofolder examples/termites_pr_eg.ll_net
./mci2dot examples/termites_pr_eg.mci > examples/termites_pr_eg.dot
dot -T pdf < examples/termites_pr_eg.dot > examples/termites_pr_eg.pdf
evince termites_pr_eg.pdf
```
Or use the workflow script `ecofold` (which contains the commands above):

```console
./ecofold examples/termites/termites_pr_eg
```

## Overview

This program implements by default the Esparza/Römer/Vogler unfolding algorithm for
low-level Petri nets (see [ERV02](#erv02)), but also one can specify to unfold according to McMillan's cutoff criterion (see [MCK93] (#mck93)). It is designed to be compatible with
the tools in the PEP project [STE01](#ste01) and with the Model-Checking Kit [SSE03](#sse03)
. Its input is a low level Petri net in PEP's .ll_net format, and its output
is the resulting unfolding in the .mci format also used in the PEP tools.

## Quick Start

Calling Ecofolder without any argument yields a usage summary.
Here is a more detailed explanation:

`-T <some_transition_name>`
  The unfolding process will abort as soon as an event is found
  which is related to the named transition.

`-d <some.number>`
  Unfolding is performed up to the given depth, i.e. the generated
  prefix contains all events whose local configuration size does
  not exceed some.number. It's likely that cutoff events won't be generated.

`-i`	The program outputs verbose information about the discovered
  events and conditions and allows the user to choose among the
  potential extensions. (Cutoffs are added immediately without
  asking because adding them at any other time would not change
  the result.)

`-r <instance>` if a marking is queried through the command `-q --query <marking>`
  then the non-negative integer number in `<instance>` will highlight the nth
  instance of the queried marking in the prefix. See `-q --query <marking>` 
  specifications for more details.

`-c` generates a compressed view of the prefix, in other words, creates a prefix with
  conditions merged if their preset and/or postset are identical.
  By doing so, one reduces tree structure of the prefix for better illustration.

`-mcmillan` shifts the default Esparza/Römer/Vogler cutoff criterion to that of McMillan. In
  short, it will consider a repeated marking if it has appeared before in the same branching process. In contrast, with the default criterion, a marking is considered already seen if it has appeared in the entire prefix (i.e., all branching processes considered).

`-freechk` checks freeness according to [GKAS24] (#gkas24). In layman words, it checks
  whether a configuration (sequence of events) is free with respect to a fate, where this fate is considered fatal for the system; being free means that one can avoid reaching this fatal state.

`-badchk <badunf>` checks badness. In other terms, given a bad net's prefix `<badunf>`
  (Unfolding prefix of a Petri net where known bad markings are initialized, so the prefix will contain all reachable and _unknown_ bad markings), it checks whether a given marking is reachable in this "bad" prefix. If yes, the marking is bad, but we cannot say that is free because it can lead to a bad marking; in this case we say that the marking is doomed.

`verbose` prints to standard output information about the net being unfolded and details
  about the prefix.

`-q --query <marking>` query a marking to highlight in the prefix. It is accompanied by an
  instance given with `-r <instance>` option. The marking must be present in the discovered markings after each extension of the unfolding process. This option does not perform a reachability check (thus is not exhaustive), for that case, you can use `check-reach` script.

`-rst --restriction <places>` restricts `<places>` to be used in the prefix unfolding.
  It can be particularly useful if the user wants to avoid the presence of certain places. Use the label of places to restrict them as input.

`-blc --block <transitions>` blocks `<transitions>` to be used in the prefix unfolding.
  It can be particularly useful if the user wants to avoid the presence of certain transitions. Use the label of transitions to block them as input.

`useids` reads id numbers given in the input `.ll_net` file. The default behavior is an
  incremental (as places are read from the file) id if this parameter is not given.

`-m <some.filename>`
  Normally, if the input net is some.ll_net, then the resulting
  unfolding will be written to some.mci. This option allows the
  result to be written to some other file.


The distribution contains an additional utility
called `mci2dot`. The input of this tool is an .mci file (as produced
by ecofolder), and its output can be processed by the [**DOT language**](https://graphviz.org/doc/info/lang.html) in
order to visualise the unfolding. Calling

```console
mci2dot some.mci
```

will cause `mci2dot` to read the file some.mci and print the results
to standard output.

### Representing a net in ll_net format

```
.ll_net format example

PEP 
PetriBox
FORMAT_N2
PL 
"p1"M1
"p2"
"p3"
TR
"r1"
"r2"
TP
1<2
2<3
PT
1>1
RS
2>2
```

Explanation:
```python
# Header
PEP 
PetriBox
FORMAT_N2
# Tag for places, a M1 at the end of each place name is added to point out is marked for the initial marking.
PL
"place_name"M1 
...
# Tag for transitions
TR
"transition_name"
...
# Tag for arcs going from transitions to places that produce. It should be interpreted as the first transition produces a token in the second place, i.e., first_transition < second_place.
TP
1<2
...
### Tag for arcs going from places to transitions that consume. It should be interpreted as the first transition consumes a token in the first place, i.e., first_place > first_transition.
PT
1>1
...
### Tag for arcs going from places to transitions that reset. It should be interpreted as the second transition resets all tokens (if any) in the second place, i.e., second_place > second_transition.
RS
2>2
...
```

## History

Previous versions of Ecofolder are [**Mole - A Petri Net Unfolder**](http://www.lsv.fr/~schwoon/tools/mole/). Ecofolder uses several contributions to the scope of unfoldings from different authors: 
  
  César Rodríguez: mci2sat.c <br/>
  Thomas Chatain, Stefan Haar, Loïg Jezequel, Loïc Paulevé, and Stefan Schowoon: allfinals.pl, attractors.py [CHJPS14](#chjps14) <br/>
  Stefan Römer, Stefan Schwoon: main core of the unfolding algorithm.

## References

Related publications:

<a id=erv02>[ERV02]</a> Javier Esparza, Stefan Römer, and Walter Vogler. An improvement of
  McMillan's unfolding algorithm. Formal Methods in System Design,
  20:285-310, 2002.

<a id=sse03>[SSE03]</a> Claus Schröter, Stefan Schwoon, and Javier Esparza. The Model-Checking
  Kit. In Wil van der Aalst and Eike Best, editors, Applications and
  Theory of Petri Nets 2003, volume 2679 of Lecture Notes in Computer
  Science, pages 463-472. Springer, June 2003. doi: https://doi.org/10.1007/3-540-44919-1_29

<a id=ste01>[STE01]</a> C. Stehno. PEP Version 2.0. Tool demonstration ICATPN 2001.
  Newcastle upon Tyne 2001.

<a id=chjps14>[CHJPS14]</a> 
  Thomas Chatain, Stefan Haar, Loïg Jezequel, Loïc Paulevé, and Stefan Schwoon. Characterization of reachable attractors using Petri net unfoldings. In Pedro Mendes, Joseph Dada, and Kieran Smallbone, editors, Computational Methods in Systems Biology, volume 8859 of Lecture Notes in Computer Science, pages 129–142. Springer Berlin Heidelberg, Cham, 2014. doi: http://dx.doi.org/10.1007/978-3-319-12982-2_10

<a id=mck93>[MCK93]</a> 
  Ken McMillan. Using unfoldings to avoid the state explosion problem in the verification of asynchronous circuits. In Gregor von Bochmann and David Karl Probst, editors, Computer Aided Verification. CAV 1992. Pages 164-177. Lecture Notes in Computer Science, vol 663. Springer, Berlin, Heidelberg. https://doi.org/10.1007/3-540-56496-9_14

<a id=gkas24>[GKAS24]</a> 
  Aguirre-Samboní, GKA, Haar, S, Paulevé, L, Schwoon, S, and Würdemann, N (2024). Attractor Basins in Concurrent Systems. arXiv preprint arXiv:2409.01079.

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see [licenses](https://www.gnu.org/licenses/).
