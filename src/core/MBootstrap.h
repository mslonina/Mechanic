#ifndef MECHANIC_BOOTSTRAP_H
#define MECHANIC_BOOTSTRAP_H

#include "MMechanic2.h"
#include "MTypes.h"
#include "MModules.h"

layer Bootstrap(int node, char *name);
layer Load(int node, char *name);
int Init(int node, layer *l);
void Finalize(int node, layer *l);
char* Filename(char *name);

#define CONFIG_OPTIONS 128
#define MECHANIC_MODULE_PREFIX "libmechanic_module_"

#if PLATFORM_DARWIN == 1
  #define LIBEXT ".dylib"
#endif

#if PLATFORM_LINUX == 1
  #define LIBEXT ".so"
#endif

#endif
