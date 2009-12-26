NAME = mpifarm
RM = rm -vf
MPICC = mpicc
MPICCFLAGS = -DWall
HDFFLAGS = -lhdf5 -lhdf5_hl
LADD = -L. -lreadconfig-static
LIBS = -lm $(HDFFLAGS)

CSOURCES = mpifarm_user.c mpifarm_skel.c
COBJECTS = $(CSOURCES:.c=.o)
OBJECTS = $(COBJECTS) 

all: lib user skel main
	
shared:
	$(MPICC) -fPIC -Wall -c libreadconfig.c
lib:
	$(MPICC) $(MPICCFLAGS) -c libreadconfig.c -o libreadconfig-static.o
	ar rcs libreadconfig-static.a libreadconfig-static.o

user:
	$(MPICC) $(MPICCFLAGS) $(LIBS) $(LADD) -c mpifarm_user.c -o mpifarm_user.o

skel:
	$(MPICC) $(MPICCFLAGS) $(LIBS) $(LADD) -c mpifarm_skel.c -o mpifarm_skel.o

main:
	$(MPICC) $(MPICCFLAGS) -o $(NAME) $(OBJECTS) $(LIBS) $(LADD)

clean:
	$(RM) mpifarm $(OBJECTS) *.a *.o
