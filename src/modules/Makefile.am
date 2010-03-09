SUBDIRS = echo mandelbrot hello
pkglib_LTLIBRARIES = mechanic_module_module.la
mechanic_module_module_la_SOURCES = mechanic_module_module.c mechanic_module_module.h
mechanic_module_module_la_LDFLAGS = -module -lm -lhdf5 -lreadconfig
mechanic_module_module_la_CFLAGS = -I. -I.. -I../core 
