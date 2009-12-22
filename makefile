all:
	mpicc mpifarm_skel.c mpifarm_user.c -o mpifarm -DWall -lhdf5 -lhdf5_hl
clean:
	rm -rf mpifarm
