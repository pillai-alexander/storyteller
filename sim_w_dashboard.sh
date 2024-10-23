#!/bin/bash

./build/sim > sim.out

Rscript figs/sim_dash.R

# rm sim.out