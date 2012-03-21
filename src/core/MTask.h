/**
 * @file
 */
#ifndef MECHANIC_TASK_H
#define MECHANIC_TASK_H

#include "MMechanic2.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MLog.h"
#include "MStorage.h"

#define TASK_FINISHED 1
#define TASK_AVAILABLE 0
#define TASK_IN_USE -1
#define NO_MORE_TASKS -99

task* TaskLoad(module *m, pool *p, int tid);
int TaskInit(module *m, pool *p, task *t);
int TaskPrepare(module *m, pool *p, task *t);
int TaskProcess(module *m, pool *p, task *t);
void TaskFinalize(module *m, pool *p, task *t);

#endif
