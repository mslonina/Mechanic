/**
 * @file
 */
#ifndef MECHANIC_TASK_H
#define MECHANIC_TASK_H

#include "MMechanic2.h"
#include "MTypes.h"

task TaskLoad(module *m, pool *p, int tid);
int TaskInit(module *m, pool *p, task *t);
int TaskPrepare(module *m, pool *p, task *t);
int TaskProcess(module *m, pool *p, task *t);
void TaskFinalize(module *m, pool *p, task *t);

#endif
