#!/bin/bash
# Sample script to run the Arnold Web module
#
# To create a dynamical map with 512x512px resolution you can try
# (with checkpoint file after 250*(mpi-size-1) computed pixels):
#
# 1) In Task Farm mode (default), 1+4 threads: 
# mpirun -n 5 mechanic2 -n arnoldweb -p arnoldweb -x 512 -y 512 -d 250
#

