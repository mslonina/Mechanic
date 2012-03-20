#ifndef MECHANIC_CHECKPOINT_H
#define MECHANIC_CHECKPOINT_H

#include "MMechanic2.h"
#include "MCommon.h"
#include "MModules.h"
#include "MLog.h"
#include "MTask.h"

checkpoint* CheckpointLoad(module *m, pool *p, int cid);
int CheckpointInit(module *m, pool *p, checkpoint *c);
int CheckpointPrepare(module *m, pool *p, checkpoint *c);
int CheckpointProcess(module *m, pool *p, checkpoint *c);
void CheckpointFinalize(module *m, pool *p, checkpoint *c);

#endif
