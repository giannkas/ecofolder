# Ecofolder - A program to unfold Petri nets with resets

## General info

Ecofolder is a software for unfolding Petri nets, its architecture is quite similar to [**Mole - A Petri Net Unfolder**](http://www.lsv.fr/~schwoon/tools/mole/). It provides a data structure for a net's unfolding and a way to visualize it in a .pdf file using the [**DOT language**](https://graphviz.org/doc/info/lang.html). Ecofolder aims to deliver means for analyzing ecosystem dynamics in nature, but its usage is not restricted to those particular systems.

## Installation

Typing `make` in the directory with the sources should do the trick. You can place the resulting executable in whichever directory you want.

You can run and see one of the examples as follows:

```console
./ecofolder examples/termites_pr_eg.ll_net
./mci2dot examples/termites_pr_eg.mci > examples/termites_pr_eg.dot
dot -T pdf < examples/termites_pr_eg.dot > examples/termites_pr_eg.pdf
evince termites_pr_eg.pdf
```

## Overview

This program implements the Esparza/Römer/Vogler unfolding algorithm for
low-level Petri nets (see [ERV02](#erv02)). It is designed to be compatible with
the tools in the PEP project [STE01](#ste01) and with the Model-Checking Kit [SSE03](#sse03)
. Its input is a low level Petri net in PEP's .ll_net format, and its output
is the resulting unfolding in the .mci format also used in the PEP tools.

## Quick Start

Calling Ecofolder without any argument yields a usage summary.
Currently, there are just four options:

`-m <some.filename>`
	Normally, if the input net is some.ll_net, then the resulting
	unfolding will be written to some.mci. This option allows the
	result to be written to some other file.

`-d <some.number>`
	Unfolding is performed up to the given depth, i.e. the generated
	prefix contains all events whose local configuration size does
	not exceed some.number. No cutoff events will be generated.

`-T <some_transition_name>`
	The unfolding process will abort as soon as an event is found
	which is related to the named transition.

`-i`	The program outputs verbose information about the discovered
	events and conditions and allows the user to choose among the
	potential extensions. (Cutoffs are added immediately without
	asking because adding them at any other time would not change
	the result.)

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
## References

Related publications:

<a id=erv02>[ERV02]</a>: Javier Esparza, Stefan Römer, and Walter Vogler. An improvement of
	McMillan's unfolding algorithm. Formal Methods in System Design,
	20:285-310, 2002.

<a id=sse03>[SSE03]</a> Claus Schröter, Stefan Schwoon, and Javier Esparza. The Model-Checking
	Kit. In Wil van der Aalst and Eike Best, editors, Applications and
	Theory of Petri Nets 2003, volume 2679 of Lecture Notes in Computer
	Science, pages 463-472. Springer, June 2003. [See also.](https://doi.org/10.1007/3-540-44919-1_29)

<a id=ste01>[STE01]</a> C. Stehno. PEP Version 2.0. Tool demonstration ICATPN 2001.
	Newcastle upon Tyne 2001.

## History

Previous versions of Ecofolder are [**Mole - A Petri Net Unfolder**](http://www.lsv.fr/~schwoon/tools/mole/), then you can check for a more detailed version history in Mole's documentation.

Version 1.0.0 (22.03.2022)
	First public release.

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