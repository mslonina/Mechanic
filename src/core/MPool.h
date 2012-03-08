#ifndef MECHANIC_POOL_H
#define MECHANIC_POOL_H

#include "MMechanic2.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MLog.h"
#include "MModules.h"
#include "MStorage.h"

pool PoolLoad(module *m, int pid);
int PoolInit(module *m, pool *p);
void PoolFinalize(pool *p);

#endif
