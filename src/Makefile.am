bin_PROGRAMS = mpifarm
mpifarm_SOURCES = module.h mpifarm.c mpifarm.h readconfig/readconfig.h
mpifarm_CPPFLAGS = -Ireadconfig/ -rdynamic 
SUBDIRS = readconfig modules
mpifarm_LDADD = readconfig/libreadconfig.a 
library_includedir=$(includedir)/mpifarm
library_include_HEADERS = mpifarm.h
