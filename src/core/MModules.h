/**
 * @file
 * The module interface header
 */
#ifndef MECHANIC_MODULES_H
#define MECHANIC_MODULES_H

#include "mechanic.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MLog.h"

#define LOAD_DEFAULT 0
#define NO_FALLBACK 1
#define FALLBACK_ONLY 2

query* LoadSym(module *m, char* name, int flag);

#endif
