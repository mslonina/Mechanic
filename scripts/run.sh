#!/bin/bash
# Sample script to run the Arnold Web module
#
# To create a dynamical map with 512x512px resolution you can try (with checkpoint file after
# 25000 computed pixels):
#
# 1) In Task Farm mode (default), 1+4 threads: 
# mpirun -n 5 mechanic -n arnoldweb -p arnoldweb -x 512 -y 512 -d 25000
#
# 2) In Masteralone mode (more than one thread in MPI spool):
# mpirun -n 5 mechanic -0 -n arnoldweb -p arnoldweb -x 512 -y 512 -d 25000
#
# 3) In Masteralone mode (one thread only in MPI spool):
# mpirun -n 1 mechanic -n arnoldweb -p arnoldweb -x 512 -y 512 -d 25000

