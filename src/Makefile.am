bin_PROGRAMS = mpifarm
mpifarm_SOURCES = module.c module.h mpifarm_user.h mpifarm_skel.c mpifarm.h readconfig/readconfig.h
mpifarm_CPPFLAGS = -Ireadconfig/ -rdynamic 
SUBDIRS = readconfig modules
mpifarm_LDADD = readconfig/libreadconfig.a 
library_includedir=$(includedir)/mpifarm
library_include_HEADERS = mpifarm.h
