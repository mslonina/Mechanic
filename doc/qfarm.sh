#!/bin/sh
#PBS -N mechanic
#PBS -l nodes=1:ppn=4:hum
#PBS -l walltime=10000:10:00
#PBS -o mechanic.out
#PBS -e mechanic.err
#PBS -q hydra

cd $PBS_O_WORKDIR
mpirun -np 4 mechanic
exit
