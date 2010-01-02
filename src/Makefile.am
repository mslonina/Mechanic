bin_PROGRAMS = mpifarm
mpifarm_SOURCES = main.c mpifarm.h master.c slave.c setup.c derived.c tools.c readconfig/readconfig.h
mpifarm_CFLAGS = -Ireadconfig/ -rdynamic 
SUBDIRS = readconfig modules
mpifarm_LDADD = readconfig/libreadconfig.a 
library_includedir=$(includedir)/mpifarm
library_include_HEADERS = mpifarm.h
