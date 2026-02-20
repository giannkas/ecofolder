#!/usr/bin/env python
import sys
import biolqm
import pypint

intput = sys.argv[1]
output = sys.argv[2]
active_nodes = sys.argv[3:]

model = biolqm.load(sys.argv[1])
an = biolqm.to_pint(model)
an.initial_state.update({a: 1 if a in active_nodes else 0 for a in an.automata})
print(an.initial_state.nonzeros())

an.export("pep", sys.argv[2])
