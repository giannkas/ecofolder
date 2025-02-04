# Ecofolder features and utilities tutorial

The next tutorial is conceived as a starting point to use all the features included in Ecofolder and postprocessing of the computed prefix.

## Table of Contents

- Prerequisites (See `readme` file)
- Installation (See `readme` file)
- [Usage](#usage)
- [Features](#features)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

## Usage

In this tutorial, we will use a toy model of a termites colony: 

**Inhabitants**:
  - Rp: reproductives
  - Wk: workers
  - Sd: soldiers
  - Te: termitomyces

**Structures**:
  - Ec: egg chambers
  - fg: fungal gardens

**Resources**:
  - Wd: wood

**Competitors**:
  - Ac: ant competitors

The rules governing this little ecosystem are:

r1. Rp+ >> Ec+, Rp+ \
r2. Rp+, Ec+ >> Wk+, Rp+, Ec+ \
r3. Wk+ >> Wd+, Te+, Fg+, Ec+, Wk+ \
r4. Wk+, Wd+ >> Sd+, Rp+, Wk+, Wd+ \
r5. Wk+, Te+ >> Wd-, Wk+, Te+ \
r6. Wd- >> Wk-, Te-, Wd- \
r7. Wk- >> Fg-, Sd-, Te-, Wk- \
r8. Wk-, Rp- >> Ec-, Wk-, Rp- \
r9. Ac+, Sd- >> Wk-, Rp-, Ac+, Sd-

In a few words, workers are responsible for creating wood, termitomyces, fungal gardens and egg chambers (r3). Reproductives also create egg chambers (r1), and both groups produce workers (r2). Workers and wood produce soldiers and reproductives (r4). From r5 onwards, there begins to be consumption of resources, reduction of inhabitants, and destruction of structures. Thus, once ants are present and soldiers are absent, there is no return to ecosystem stabilization because workers will be annihilated, and so will reproductives (r9).

In `examples/termites` folder you will find `termites.ll_net` and `termites_pr.ll_net` files. Both files correspond to the aforementioned example, the second one is after applying the place-replication encoding (see [Efficient unfolding of contextual Petri nets](https://www.sciencedirect.com/science/article/pii/S0304397512004318?via%3Dihub)) 

### Displaying nets

You can visualize both nets with the next command (use `ecofolder` as parent folder):

Original net:
`./shownet examples/termites/tutorial/termites`

After PR-encoding (`./prencoding examples/termites/tutorial/termites.ll_net`)
`./shownet examples/termites/tutorial/termites_pr`

Where blue arcs represent read arcs (see [Efficient unfolding of contextual Petri nets](https://www.sciencedirect.com/science/article/pii/S0304397512004318?via%3Dihub))

### Unfolding nets

Let's unfold the termites example (`termites_pr.ll_net`). The reason why we use the example after the place-replication encoding is because we need to represent the dynamics of a contextual-reset net in an ordinary occurrence net through an unfolding procedure that accounts for read (and reset) arcs (see [Ecosystem causal analysis using Petri net unfoldings](https://theses.hal.science/tel-04556225/document)).

```bash
./src/ecofolder examples/termites/tutorial/termites_pr.ll_net
./src/mci2dot examples/termites/tutorial/termites_pr_unf.mci > examples/termites/tutorial/termites_pr_unf.dot
dot -T pdf examples/termites/tutorial/termites_pr_unf.dot > examples/termites/tutorial/termites_pr_unf.pdf
evince examples/termites/tutorial/termites_pr_unf.pdf
```

Or you can collate the four commands and run the script:

```bash
./ecofold examples/termites/tutorial/termites_pr
```

The ouput will display a big prefix of the unfolding which is stopped by a cutoff criterion, here we refer as the _Esparza criterion_ (see [An improvement of McMillan's unfolding algorithm](https://link.springer.com/content/pdf/10.1023/A:1014746130920.pdf)). However, you can also run with another one, which we refer as the _McMillan criterion_ (see [Using unfoldings to avoid the state explosion problem
in the verification of asynchronous circuits](https://link.springer.com/chapter/10.1007/3-540-56496-9_14)).

```bash
./src/ecofolder -mcmillan examples/termites/tutorial/termites_pr.ll_net
```
### Compressing or merging conditions in the prefix

All of the prefixes generated so far are big enough that makes it difficult to see in detail. One can try to merge conditions (circles) according to their pre- postsets. The next commands achieve a more compact view:

```bash
./src/ecofolder -c examples/termites/tutorial/termites_pr.ll_net
./src/mci2dot_cpr examples/termites/tutorial/termites_pr_unf.mci > examples/termites/tutorial/termites_pr_unf.dot
dot -T pdf examples/termites/tutorial/termites_pr_unf.dot > examples/termites/tutorial/termites_pr_unf.pdf
evince examples/termites/tutorial/termites_pr_unf.pdf
```
### Stopping unfolding process

You can also decide where to stop the unfolding process _before_ all cutoff events have truncated all branching processes either by:

1. Stopping when a transition is inserted, for instance:

```bash
./src/ecofolder -T "R5" examples/termites/tutorial/termites_pr.ll_net
./src/mci2dot examples/termites/tutorial/termites_pr_unf.mci > examples/termites/tutorial/termites_pr_unf.dot
dot ...
evince ...
```

2. Or halting according to a certain depth, with respect to the number of times a triple of cond-event (first level) (cond-event (second level), cond-event (third level), and so forth)  has been added.

```bash
./src/ecofolder -d 5 examples/termites/tutorial/termites_pr.ll_net
./src/mci2dot examples/termites/tutorial/termites_pr_unf.mci > examples/termites/tutorial/termites_pr_unf.dot
dot ...
evince ...
```
### Interactive and maximal configurations mode

If one want to manipulate the unfolding process and see how it goes step by step, an _interactive mode_ can be enabled:

```bash
./src/ecofolder -i examples/termites/tutorial/termites_pr.ll_net
./src/mci2dot ...
dot ...
evince ...
```

Moreover, one can enable a mode where an individual branching process is unfolded until it's stopped by a cutoff event (note that in this case both Esparza and McMillan criteria produce the same output since we only consider a branching process and not all). In other words, we unfold until we find a marking already visited (or a _loop_ 🙂).

```bash
./src/ecofolder -confmax examples/termites/tutorial/termites_pr.ll_net
./src/mci2dot ...
dot ...
evince ...
```

### Querying markings in the prefix

One can also query a marking that appear in the prefix to be highlighted. The marking must have been used in the cutoff procedure to be found, otherwise it will not be highlighted (note that it's not a _reachability check_).

Say that we want to highlight this marking:

`Ac+,Ec-,Fg-,Rp-,Sd-,Te-,Wd+,Wk-`

So we proceed as follows (`-q` is to add a marking to be queried and `-r` is to say that we want to highlight certain instance, eg. `-r 1` is the first instance, and `-r 0` is to highlight all instances):

```bash
./src/ecofolder -q Ac+,Ec-,Fg-,Rp-,Sd-,Te-,Wd+,Wk- examples/termites/tutorial/termites_pr.ll_net
./src/mci2dot -r 0 examples/termites/tutorial/termites_pr_unf.mci > examples/termites/tutorial/termites_pr_unf.dot
dot ...
evince ...
```

### Restricting places and blocking transitions

One can restrict a place to appear (say you don't want a place to be used, i.e. to receive tokens):

```bash
./src/ecofolder -rst Wd-,Wk- examples/termites/tutorial/termites_pr.ll_net
./src/mci2dot examples/termites/tutorial/termites_pr_unf.mci > examples/termites/tutorial/termites_pr_unf.dot
dot ...
evince ...
```

In this case, the absence of wood and workers cannot have tokens, so only rules (or transitions) that do not put tokens in those places can be used. Similarly, one can block transitions to be fired:

```bash
./src/ecofolder -blc R1,R2,R3,R4,R5 examples/termites/tutorial/termites_pr.ll_net
./src/mci2dot examples/termites/tutorial/termites_pr_unf.mci > examples/termites/tutorial/termites_pr_unf.dot
dot ...
evince ...
```

### Export to CSV format

One can export the prefix structure to be expressed in comma-separated values as follows:

```bash
./src/mci2csv examples/termites/tutorial/termites_pr_unf.mci
```

This command will output two files: `termites_pr_unf_nodes.csv` and `termites_pr_unf_ends.csv`.

**`termites_pr_unf_nodes.csv`**

A file formatted with five columns containing the node id, its type, its name or label, number of tokens (if any) and whether the node is a cutoff event (empty field for conditions). For example:

```csv
id,type,name,tokens,cutoff
"c1","condition","Wk-_2","0",
"c2","condition","Rp+_2","0",
"c3","condition","Wd+","0",
"e21","event","R6",,"0"
"e22","event","R2",,"0"
"e23","event","R9",,"1"
```

**`termites_pr_unf_ends.csv`**

A file formatted with three columns containing edge type, id node source and id node destination. For example:

```csv
type,src,dst
"edge","c1","e11"
"edge","c1","e60"
"edge","c1","e8"
"edge","c1","e7"
"edge","e68","c45"
"edge","e68","c46"
"edge","e3","c47"
"edge","c47","e11"
"edge","c47","e60"
```

Alternatively, one can generate a CSV file in case conditions are merged (compressed) in `.mci` file. The next command achieves that:

```bash
./src/mci2csv -c examples/termites/tutorial/termites_pr_unf.mci
```

Which will generate the same files as before but changing some fields in `termites_pr_unf_nodes.csv` when conditions are packed in the same entry. For example,

```csv
id,type,name,tokens,cutoff
"c1,c2","condition","Wk-_1,Wk-_2","0,0",
"c3,c4","condition","Rp+_1,Rp+_2","0,0",
"c5","condition","Wd+","0",
"c6,c7","condition","Fg-,Fg+","1,0",
"c175,c176,c177","condition","Fg+,Sd+,Fg-","0,0,1",
"e1","event","R3",,"0"
"e2","event","R6",,"0"
"e3","event","R4",,"0"
"e4","event","R5",,"0"
```

The output with multiple conditions on the same line should be interpreted as follows: each condition feature is listed in the order they appear, e.g., `"c1,c2","condition","Wk-_1,Wk-_2","0,0",`. The label for `c1` is `Wk-_1` and it has `0` tokens. Both nodes are of `condition` type.

In addition, CSV files can be generated from an event structure created by `mci2dot_ev` module, the following command do this task from a `.mci` file (note that the prefix saved as binary in `mci` has to be generated without merging conditions: `./src/ecofolder examples/termites/tutorial/termites_pr.ll_net`):

```bash
./src/mci2dot_ev -csv examples/termites/tutorial/termites_pr_unf.mci
```

The command creates the two files, but with a slight modification to include conflicts but no conditions:

**`termites_pr_unf_nodes.csv`**

```csv
id,type,name,tokens,cutoff
"e20","event","R5",,"0"
"e21","event","R6",,"0"
"e22","event","R2",,"0"
"e23","event","R9",,"1"
"e24","event","R6",,"1"
"e25","event","R3",,"1"
```

**`termites_pr_unf_ends.csv`**

```csv
type,src,dst
"edge","e1","e3"
"edge","e1","e4"
"edge","e1","e5"
"edge","e0","e2"
"edge","e0","e68"
"conflict","e1","e2"
"conflict","e1","e68"
"conflict","e2","e68"
```


## Installation

Follow these steps to install the project:

1. Clone the repository:
   ```bash
   git clone https://github.com/yourusername/yourproject.git


The distribution contains an additional utility
called `mci2dot`. The input of this tool is an .mci file (as produced
by ecofolder), and its output can be processed by the [**DOT language**](https://graphviz.org/doc/info/lang.html) in
order to visualise the unfolding. Calling

```console
mci2dot some.mci
```

will cause `mci2dot` to read the file some.mci and print the results
to standard output.