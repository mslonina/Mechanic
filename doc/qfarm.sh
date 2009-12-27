#!/bin/sh
#PBS -N mpifarm
#PBS -l nodes=1:ppn=4:hum
#PBS -l walltime=10000:10:00
#PBS -o mpifarm.out
#PBS -e mpifarm.err
#PBS -q hydra

cd $PBS_O_WORKDIR
mpirun -np 4 mpifarm
exit
