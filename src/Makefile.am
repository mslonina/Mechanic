bin_PROGRAMS = mpifarm
mpifarm_SOURCES = mpifarm_user.c mpifarm_skel.c mpifarm_user.h mpifarm_skel.h readconfig/readconfig.h
mpifarm_CPPFLAGS = -Ireadconfig/
SUBDIRS = readconfig
mpifarm_LDADD = readconfig/libreadconfig.la
