#ifndef MECHANIC_BOOTSTRAP_H
#define MECHANIC_BOOTSTRAP_H

#include "MMechanic2.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MModules.h"

module Bootstrap(int node, char *name, module *f);
module Load(int node, char *name);
int Init(int node, module *m);
int Setup(int node, module *m);
void FinalizeLayer(int node, layer *l);
void Finalize(int node, module *m);

#define MECHANIC_MODULE_PREFIX "libmechanic_module_"

#if PLATFORM_DARWIN == 1
  #define LIBEXT ".dylib"
#endif

#if PLATFORM_LINUX == 1
  #define LIBEXT ".so"
#endif

#endif
