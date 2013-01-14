/**
 * @file
 * The core-only functions
 */
#ifndef MECHANIC_CORE_H
#define MECHANIC_CORE_H

#include "mechanic.h"
#include "MTypes.h"
#include "MLog.h"
#include "MCommon.h"
#include "MModules.h"

#define CORE_MODULE "core"

void Welcome(void);
int Prepare(module *m);
int Process(module *m, pool **all);
int NodePrepare(module *m, pool **all, pool *current);
int NodeProcess(module *m, pool **all, pool *current);
int LoopPrepare(module *m, pool **all, pool *current);
int LoopProcess(module *m, pool **all, pool *current);
int Send(int node, int dest, int tag, module *m, pool *p);
int Receive(int node, int sender, int tag, module *m, pool *p, void *buffer);

#endif
