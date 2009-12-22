#!/bin/sh
#PBS -N mpimaps
#PBS -l nodes=2:ppn=2:hydra
#PBS -l walltime=10000:10:00
#PBS -o mpimaps.out
#PBS -e mpimaps.err
#PBS -q hydra

cd $PBS_O_WORKDIR
./mpimaps
exit
