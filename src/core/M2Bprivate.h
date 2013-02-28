/**
 * @file
 * The bootstrap stage
 */
#ifndef MECHANIC_M2B_PRIVATE_H
#define MECHANIC_M2B_PRIVATE_H

#include "M2Aprivate.h"
#include "M2Epublic.h"
#include "M2Cprivate.h"
#include "M2CSprivate.h"
#include "M2Hpublic.h"
#include "M2Mpublic.h"

module Bootstrap(int node, int mpisize, int argc, char **argv, char *name, module *f);
module ModuleLoad(char *name);
void* RuntimeModeLoad(char *name);
int ModuleInit(module *m);
int ModuleSetup(module *m, int argc, char **argv);
void FinalizeLayer(layer *l);
void ModuleFinalize(module *m);

#define MECHANIC_MODULE_PREFIX "libmechanic_module_"
#define MECHANIC_MODE_PREFIX "libmechanic_mode_"

#if PLATFORM_DARWIN == 1
  #define LIBEXT ".dylib"
#endif

#if PLATFORM_LINUX == 1
  #define LIBEXT ".so"
#endif

#endif
