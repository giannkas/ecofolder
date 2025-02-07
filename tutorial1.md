## Tools

Ecofolder comes with some python modules/tools to make computations based on unfoldings, for instance: computing attractors (see [Characterization of reachable attractors using Petri net unfoldings](https://hal.science/hal-01060450/file/unfolding_for_attractors.pdf)) and their basins (or also minimally doomed configurations) (see [Attractor basins in concurrent systems](https://arxiv.org/pdf/2409.01079)). Loosely speaking, attractors are equivalent to the _terminal_ strongly connected components of a system, so it's interesting to characterize them and identify where our system may loop forever. _Partial_ attractor basins or minimally doomed configurations, on the other hand, refer to sequences of events that may lead into an attractor, but if the last event of this sequence is removed, then we can "steer" away from the attractor, potentially avoiding a fatal outcome for the system.

# Attractors

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

# Minimally doomed configurations

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

