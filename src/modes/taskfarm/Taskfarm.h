#ifndef MECHANIC_MODE_TASKFARM_H
#define MECHANIC_MODE_TASKFARM_H

#include "mechanic.h"
#include "MCommon.h"
#include "MTypes.h"
#include "MPool.h"
#include "MMpi.h"
#include "MCheckpoint.h"
#include "MTask.h"
#include "MCore.h"
#include "MRestart.h"

int Master(module *m, pool *p);
int Worker(module *m, pool *p);

#endif
