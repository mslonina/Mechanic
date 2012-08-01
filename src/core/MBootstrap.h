#ifndef MECHANIC_BOOTSTRAP_H
#define MECHANIC_BOOTSTRAP_H

#include "Mechanic2.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MModules.h"
#include "MSetup.h"

module Bootstrap(int node, int mpisize, int argc, char **argv, char *name, module *f);
module ModuleLoad(char *name);
int ModuleInit(module *m);
int ModuleSetup(module *m, int argc, char **argv);
void FinalizeLayer(layer *l);
void ModuleFinalize(module *m);

#define MECHANIC_MODULE_PREFIX "libmechanic_module_"

#if PLATFORM_DARWIN == 1
  #define LIBEXT ".dylib"
#endif

#if PLATFORM_LINUX == 1
  #define LIBEXT ".so"
#endif

#endif
