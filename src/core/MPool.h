/**
 * @file
 * The Pool interface header
 */
#ifndef MECHANIC_POOL_H
#define MECHANIC_POOL_H

#include "mechanic.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MLog.h"
#include "MModules.h"
#include "MStorage.h"
#include "MMpi.h"

pool* PoolLoad(module *m, int pid);
int PoolPrepare(module *m, pool **all, pool *p);
int PoolProcess(module *m, pool **all, pool *p);
int PoolReset(module *m, pool *p);
void PoolFinalize(module *m, pool *p);

int PoolProcessData(module *m, pool *p, setup *s);

#endif
