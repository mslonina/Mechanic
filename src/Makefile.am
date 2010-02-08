SUBDIRS = core engines modules modes
bin_PROGRAMS = mechanic
library_includedir=$(includedir)
library_include_HEADERS = core/mechanic.h
mechanic_CFLAGS = -rdynamic -Icore 
mechanic_SOURCES = mechanic.c
mechanic_LDADD = core/libmechanic_core.a \
								 engines/gpu/libmechanic_engine_gpu.a \
								 engines/odex/libmechanic_engine_odex.a \
								 engines/taylor/libmechanic_engine_taylor.a \
								 modes/farm/libmechanic_mode_farm.a \
								 modes/multifarm/libmechanic_mode_multifarm.a \
								 modes/masteralone/libmechanic_mode_masteralone.a 
