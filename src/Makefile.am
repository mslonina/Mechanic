bin_PROGRAMS = mpifarm
mpifarm_SOURCES = main.c mpifarm.h mpifarm-internals.h master.c slave.c hdf5.c setup.c derived.c tools.c
mpifarm_CFLAGS = -rdynamic 
mpifarm_LDFLAGS = -L/home/slonina/usr/lib/libreadconfig -lreadconfig 
SUBDIRS = modules
library_includedir=$(includedir)
library_include_HEADERS = mpifarm.h
