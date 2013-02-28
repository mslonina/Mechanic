/**
 * @file
 * The task pool
 */
#ifndef MECHANIC_M2P_PRIVATE_H
#define MECHANIC_M2P_PRIVATE_H

#include "M2Ppublic.h"

pool* PoolLoad(module *m, int pid);
int PoolPrepare(module *m, pool **all, pool *p);
int PoolProcess(module *m, pool **all, pool *p);
int PoolReset(module *m, pool *p);
void PoolFinalize(module *m, pool *p);

int PoolProcessData(module *m, pool *p, setup *s);

#endif
