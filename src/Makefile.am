bin_PROGRAMS = mpifarm
mpifarm_SOURCES = mpifarm_user.c mpifarm_skel.c mpifarm.h readconfig/readconfig.h
mpifarm_CPPFLAGS = -Ireadconfig/ -rdynamic -g
SUBDIRS = readconfig modules
mpifarm_LDADD = readconfig/libreadconfig.a 
library_includedir=$(includedir)/mpifarm
library_include_HEADERS = mpifarm.h
