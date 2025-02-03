# Ecofolder and utilities tutorial

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