#ifndef MECHANIC_POOL_H
#define MECHANIC_POOL_H

#include "MMechanic2.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MLog.h"
#include "MModules.h"
#include "MStorage.h"
#include "MMpi.h"

pool* PoolLoad(module *m, int pid);
int PoolPrepare(module *m, pool **all, pool *p);
int PoolProcess(module *m, pool **all, pool *p);
void PoolFinalize(module *m, pool *p);

#endif