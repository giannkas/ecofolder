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

r1. Rp+ >> Ec+, Rp+

r2. Rp+, Ec+ >> Wk+, Rp+, Ec+
r3. Wk+ >> Wd+, Te+, Fg+, Ec+, Wk+
r4. Wk+, Wd+ >> Sd+, Rp+, Wk+, Wd+
r5. Wk+, Te+ >> Wd-, Wk+, Te+
r6. Wd- >> Wk-, Te-, Wd-
r7. Wk- >> Fg-, Sd-, Te-, Wk-
r8. Wk-, Rp- >> Ec-, Wk-, Rp-
r9. Ac+, Sd- >> Wk-, Rp-, Ac+, Sd-


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