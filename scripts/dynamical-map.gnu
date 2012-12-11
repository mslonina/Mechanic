#!/usr/bin/gnuplot -persist
#
# Template gnuplot script to plot dynamical map from MECHANIC 
# 
# 1) Run the Mechanic:
# mpirun -np 5 mechanic -p arnoldweb -n arnoldweb -x 512 -y 512 -d 250
# 2) Extract data:
# h52ascii arnoldweb-master-00.h5 512 /Pools/pool-0000/Tasks/result pm3d.dat
# 3) Run Gnuplot:
# gnuplot plot.gnu
#
set pm3d map 
unset key
set size square
set xlabel "action I_1"
set ylabel "action I_2"
set term post portrait enhanced color "Helvetica" 14
set out "arnoldweb.eps"
set title "Arnold web (test plot)"
max = 5
splot [0.8:1.2][0.8:1.2][0:max] 'pm3d.dat' u 1:2:($3<max?abs($3):max)
set out

