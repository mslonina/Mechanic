#ifndef MECHANIC_MODE_TASKFARM_H
#define MECHANIC_MODE_TASKFARM_H

#include "MMechanic2.h"
#include "MCommon.h"
#include "MTypes.h"
#include "MLog.h"
#include "MPool.h"
#include "MMpi.h"
#include "MCheckpoint.h"
#include "MTask.h"

int Taskfarm(module *m);
int Master(module *m, pool *p);
int Worker(module *m, pool *p);

#endif
