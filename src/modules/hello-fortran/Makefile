MNAME=ff
LDFLAGS= -shared
FFLAGS= -fPIC
MYFLAGS= -std=f2003 -c
FF=gfortran

all: bind module
	${FF} ${LDFLAGS} mechanic_module_${MNAME}.o -o libmechanic_module_${MNAME}.so

bind:
	${FF} ${MYFLAGS} mechanic_fortran.f03

module:
	${FF} ${FFLAGS} ${MYFLAGS} mechanic_module_${MNAME}.f03 -o mechanic_module_${MNAME}.o

clean:
	rm -rf *.o *.mod

distclean: clean
	rm -rf *.so
