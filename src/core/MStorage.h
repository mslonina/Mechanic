#ifndef MECHANIC_STORAGE_H
#define MECHANIC_STORAGE_H

#include "MMechanic2.h"
#include "MLog.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MModules.h"

int CommitStorageLayout(hid_t location, storage *s);
int CommitMemoryLayout(storage *s);
void FreeMemoryLayout(storage *s);
int Storage(module *m, pool *p);

int CommitData(hid_t location, storage *s, double **data);
int WritePoolData(pool *p);

int GetBanks(storage *s);



#endif
