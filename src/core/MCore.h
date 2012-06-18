/**
 * @file
 */
#ifndef MECHANIC_CORE_H
#define MECHANIC_CORE_H

#include "MMechanic2.h"
#include "MTypes.h"
#include "MLog.h"
#include "MCommon.h"
#include "MModules.h"

#define CORE_MODULE "core"

void Welcome();
int Prepare(module *m);
int Process(module *m, pool **all);

#endif
