MNAME=ff
LDFLAGS= -shared
FFLAGS=-fPIC
MYFLAGS=-std=f2003 -c

all: bind module
	gfortran ${LDFLAGS} mechanic_module_${MNAME}.o -o libmechanic_module_${MNAME}.so

bind:
	gfortran ${MYFLAGS} mechanic_fortran.f03

module:
	gfortran ${FFLAGS} ${MYFLAGS} mechanic_module_${MNAME}.f03 -o mechanic_module_${MNAME}.o

clean:
	rm -rf *.o *.mod

distclean: clean
	rm -rf *.so
