/**
 * @file
 * Module handlers (public API) 
 */
#ifndef MECHANIC_M2H_PUBLIC_H
#define MECHANIC_M2H_PUBLIC_H

#include <dlfcn.h>

#include "M2Apublic.h"
#include "M2Epublic.h"

#define LOAD_DEFAULT 0
#define NO_FALLBACK 1
#define FALLBACK_ONLY 2

query* LoadSym(module *m, char* name, int flag);

#endif
