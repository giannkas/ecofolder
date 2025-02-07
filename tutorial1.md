## Tools

Ecofolder comes with some python modules/tools to make computations based on unfoldings, for instance: computing attractors (see [Characterization of reachable attractors using Petri net unfoldings](https://hal.science/hal-01060450/file/unfolding_for_attractors.pdf)) and their basins (or also minimally doomed configurations) (see [Attractor basins in concurrent systems](https://arxiv.org/pdf/2409.01079)). Loosely speaking, attractors are equivalent to the _terminal_ strongly connected components of a system, so it's interesting to characterize them and identify where our system may loop forever. _Partial_ attractor basins or minimally doomed configurations, on the other hand, refer to sequences of events that may lead into an attractor, but if the last event of this sequence is removed, then we can "steer" away from the attractor, potentially avoiding a fatal outcome for the system.

### Attractors

We can compute the attractors for our termites colony example with the following command:

```bash
./attractors examples/termites/tutorial/termites_pr
```

which will output:

```bash
### filtering 5 (6) final markings ...
...++
Ac+ Ec- Fg- Rp- Sd- Te- Wd+ Wk-_1 Wk-_2
Ac+ Ec- Fg- Rp- Sd- Te- Wd- Wk-_1 Wk-_2
```

This means that there are two attractors in the net for the initial marking we used (Ac+, Ec-, Fg-, Rp-, Sd-, Te-, Wd-, Wk+). The fact that we only show a marking for each attractor does not imply there is one marking within the attractor; it may contain more. You can check the unfolding prefix of each attractor with the already known routine:

```bash
./src/mci2dot examples/termites/tutorial/termites_pr-a0.mci > examples/termites/tutorial/termites_pr-a0_unf.dot
dot -T pdf examples/termites/tutorial/termites_pr-a0_unf.dot > examples/termites/tutorial/termites_pr-a0_unf.pdf
evince examples/termites/tutorial/termites_pr-a0_unf.pdf
```

### Minimally doomed configurations

One can compute these configurations by defining a list of undesired or _bad_ markings which we would like to avoid, say for example the next marking:

```bash
Ac+,Ec*,Fg*,Rp-,Sd*,Te*,Wd*,Wk-
```
> [!NOTE]
> `*` is a placeholder for `+` and `-`, in other words, we include both combinations of using `+` or `-` in the corresponding place. We consider the folowing list of markings (up to 32 combinations, 2^(number of star variables)):
>
> ```bash
> Ac+,Ec+,Fg+,Rp-,Sd+,Te+,Wd+,Wk-
> Ac+,Ec+,Fg+,Rp-,Sd+,Te+,Wd-,Wk-
> Ac+,Ec+,Fg+,Rp-,Sd+,Te-,Wd+,Wk-
> Ac+,Ec+,Fg+,Rp-,Sd+,Te-,Wd-,Wk-
> Ac+,Ec+,Fg+,Rp-,Sd-,Te+,Wd+,Wk-
> Ac+,Ec+,Fg+,Rp-,Sd-,Te+,Wd-,Wk-
> Ac+,Ec+,Fg+,Rp-,Sd-,Te-,Wd+,Wk-
> Ac+,Ec+,Fg+,Rp-,Sd-,Te-,Wd-,Wk-
> Ac+,Ec+,Fg-,Rp-,Sd+,Te+,Wd+,Wk-
> Ac+,Ec+,Fg-,Rp-,Sd+,Te+,Wd-,Wk-
> ...
> ```

We can then compute the minimally doomed configurations using these bad markings with the following command:

`./doomed examples/termites/tutorial/termites_pr examples/termites/tutorial/termites_pr.bad`

And the following 11 configurations will be printed (which can be printed in different order):

```bash
Prefix has 68 events, including cut-offs
Unchallenged events {'(pi0,e39)', '(pi0,e40)'}
prefix_d DiGraph with 67 nodes and 153 edges
minFO: 23it [00:00, 139.23it/s]
00m00s [MINDOO 1]        ['R3', 'R4', 'R5', 'R6', 'R7', 'R2', 'R3', 'R5', 'R6', 'R2', 'R9'] ['(pi0,e1)', '(pi0,e3)', '(pi0,e6)', '(pi0,e11)', '(pi0,e16)', '(pi0,e18)', '(pi0,e19)', '(pi0,e20)', '(pi0,e21)', '(pi0,e22)', '(pi0,e23)']
Marking:  ('Rp-', 'Wk-_1', 'Sd-', 'Te-', 'Ec+', 'Fg+', 'Wd-', 'Ac+', 'Wk-_2')
   free checks: 5
00m00s [MINDOO 2]        ['R3', 'R4', 'R5', 'R6', 'R7', 'R2', 'R9'] ['(pi0,e1)', '(pi0,e3)', '(pi0,e6)', '(pi0,e11)', '(pi0,e16)', '(pi0,e18)', '(pi0,e36)']
Marking:  ('Rp-', 'Wk-_1', 'Sd-', 'Te-', 'Ec+', 'Fg-', 'Wd-', 'Ac+', 'Wk-_2')
   free checks: 7
00m00s [MINDOO 3]        ['R3', 'R5', 'R6'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e7)']
Marking:  ('Rp-', 'Wk-_1', 'Sd-', 'Te-', 'Ec+', 'Fg+', 'Wd-', 'Ac+', 'Wk-_2')
   free checks: 11
00m00s [MINDOO 4]        [] []
Marking:  ('Rp-', 'Wk+_1', 'Sd-', 'Te-', 'Ec-', 'Fg-', 'Wd-', 'Ac+', 'Wk+_2', 'Wk+_3')
   free checks: 12
00m00s [MINDOO 5]        ['R3', 'R4', 'R5', 'R6', 'R7', 'R2', 'R3', 'R5', 'R6', 'R9'] ['(pi0,e1)', '(pi0,e3)', '(pi0,e6)', '(pi0,e11)', '(pi0,e16)', '(pi0,e18)', '(pi0,e19)', '(pi0,e20)', '(pi0,e21)', '(pi0,e27)']
Marking:  ('Rp-', 'Wk-_1', 'Sd-', 'Te-', 'Ec+', 'Fg+', 'Wd-', 'Ac+', 'Wk-_2')
   free checks: 16
00m00s [MINDOO 6]        ['R3', 'R4', 'R5', 'R6', 'R7', 'R9'] ['(pi0,e1)', '(pi0,e3)', '(pi0,e6)', '(pi0,e11)', '(pi0,e16)', '(pi0,e43)']
Marking:  ('Rp-', 'Wk-_1', 'Sd-', 'Te-', 'Ec+', 'Fg-', 'Wd-', 'Ac+', 'Wk-_2')
   free checks: 19
00m00s [MINDOO 7]        ['R3', 'R4', 'R5', 'R6', 'R7', 'R2', 'R3', 'R9'] ['(pi0,e1)', '(pi0,e3)', '(pi0,e6)', '(pi0,e11)', '(pi0,e16)', '(pi0,e18)', '(pi0,e19)', '(pi0,e32)']
Marking:  ('Rp-', 'Wk-_1', 'Sd-', 'Te+', 'Ec+', 'Fg+', 'Wd+', 'Ac+', 'Wk-_2')
   free checks: 21
00m01s [MINDOO 8]        ['R3', 'R4', 'R5', 'R6', 'R7', 'R2', 'R3', 'R5', 'R9'] ['(pi0,e1)', '(pi0,e3)', '(pi0,e6)', '(pi0,e11)', '(pi0,e16)', '(pi0,e18)', '(pi0,e19)', '(pi0,e20)', '(pi0,e29)']
Marking:  ('Rp-', 'Wk-_1', 'Sd-', 'Te+', 'Ec+', 'Fg+', 'Wd-', 'Ac+', 'Wk-_2')
   free checks: 28
00m01s [MINDOO 9]        ['R6'] ['(pi0,e2)']
Marking:  ('Rp-', 'Wk-_1', 'Sd-', 'Te-', 'Ec-', 'Fg-', 'Wd-', 'Ac+', 'Wk-_2')
   free checks: 34
00m01s [MINDOO 10]       ['R3', 'R5', 'R9'] ['(pi0,e1)', '(pi0,e4)', '(pi0,e8)']
Marking:  ('Rp-', 'Wk-_1', 'Sd-', 'Te+', 'Ec+', 'Fg+', 'Wd-', 'Ac+', 'Wk-_2')
   free checks: 41
00m01s [MINDOO 11]       ['R3', 'R9'] ['(pi0,e1)', '(pi0,e5)']
Marking:  ('Rp-', 'Wk-_1', 'Sd-', 'Te+', 'Ec+', 'Fg+', 'Wd+', 'Ac+', 'Wk-_2')
   free checks: 43
total free checks: 44
```

A noteworthy configuration is the 4th because it is empty. An empty doomed configuration means that the initial marking can lead you to the collapse, besides of being minimal, it suggests that you may not have option to prevent the ecosystem downfall, and then yo should change the initial state/marking.

(On another software release, we plan to provide illustrations of these minimally doomed configurations).

### Reachability checks

One can perform reachability checks using the scripts `check-reach` and `check-reach_from-mci`. The first script unfolds a net, while the second operates from the prefix onward.

For example:

```bash
./check-reach examples/termites/tutorial/termites_pr Ac+,Ec+,Fg+,Rp+,Sd-,Te+,Wd+,Wk+_1,Wk+_2,Wk+_3
```

Whose output is:

```bash
Unfolding...
Translation to SAT...
SAT checking...
Evaluating SAT...
Marking reachable:
 R3 (e1) R4 (e3) R5 (e6) R6 (e11) R7 (e16) R2 (e18) R3 (e19)
        Rp+_1 (c49) Wk+_1 (c389) Sd- (c244) Te+ (c393) Ec+ (c392) Fg+ (c390) Wd+ (c391) Ac+ (c12) Rp+_2 (c332) Wk+_2 (c336) Wk+_3 (c335) 
Converting to dot...
Producing the pdf...
Displaying the pdf...
```

And a PDF file is displayed. These reachability checks are precisely for checking whether a marking (from the initial marking) is reachable in the net. The output only verifies for a unique event sequence that reaches the target marking, however, there could be more.

