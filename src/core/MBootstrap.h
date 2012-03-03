#ifndef MECHANIC_BOOTSTRAP_H
#define MECHANIC_BOOTSTRAP_H

#include "MMechanic2.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MModules.h"

module Bootstrap(int node, char *name, module *f);
module ModuleLoad(int node, char *name);
int ModuleInit(int node, module *m);
int ModuleSetup(int node, module *m);
void FinalizeLayer(int node, layer *l);
void ModuleFinalize(int node, module *m);

#define MECHANIC_MODULE_PREFIX "libmechanic_module_"

#if PLATFORM_DARWIN == 1
  #define LIBEXT ".dylib"
#endif

#if PLATFORM_LINUX == 1
  #define LIBEXT ".so"
#endif

#endif
