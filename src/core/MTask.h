/**
 * @file
 * The task interface header
 */
#ifndef MECHANIC_TASK_H
#define MECHANIC_TASK_H

#include "MMechanic2.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MLog.h"
#include "MStorage.h"

task* TaskLoad(module *m, pool *p, int tid);
int TaskPrepare(module *m, pool *p, task *t);
int TaskProcess(module *m, pool *p, task *t);
void TaskReset(module *m, pool *p, task *t, int tid);
void TaskFinalize(module *m, pool *p, task *t);

#endif
