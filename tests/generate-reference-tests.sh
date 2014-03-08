#!/bin/bash

MODULES=(
  core
  #tex_attr
  #tex_changepoollayout
  #tex_changepoollayout2
  #tex_chreset
  #tex_compound
  #tex_compound_attr
  #tex_createpool
  #tex_datatypes
  #tex_dim
  #tex_dset
  #tex_loop
  #tex_mandelbrot
  #tex_map
  #tex_node
  #tex_pool
  #tex_poolmask
  #tex_poolsetup
  #tex_poolsetup2
  #tex_poolsize
  #tex_prepareprocess
  #tex_readfile
  #tex_readfile_setup
  #tex_reset
  #tex_setup
  #tex_stage
  #tex_taskcheckpoint
)

# master mode
for MODULE in ${MODULES[*]}; do
  mpirun -np 4 mechanic -m master -p $MODULE -n $MODULE-master-mode -x 10 -y 10 -b 3 -d 13 --test --restart-file=$MODULE-master-mode-master-02.h5
done

# task farm mode
for MODULE in ${MODULES[*]}; do
  mpirun -np 4 mechanic -m taskfarm -p $MODULE -n $MODULE -x 10 -y 10 -b 3 -d 13 --test --restart-file=$MODULE-master-02.h5
done
