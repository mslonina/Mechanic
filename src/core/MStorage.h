#ifndef MECHANIC_STORAGE_H
#define MECHANIC_STORAGE_H

#include "MMechanic2.h"
#include "MLog.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MModules.h"

#define STORAGE_FULL 1
#define STORAGE_DEFAULT 0

int CommitStorageLayout(hid_t location, int banks, storage *s);
int CommitMemoryLayout(int banks, storage *s);
void FreeMemoryLayout(int banks, storage *s);
int Storage(module *m, pool *p);
int CheckLayout(int banks, storage *s);

int CommitDataset(hid_t location, storage *s, double **data);
int CommitData(hid_t location, int banks, storage *s, int flags);
int WritePoolData(pool *p);

int GetBanks(int allocated_banks, storage *s);

storage* StorageLoad(int banks);
void StorageFinalize(int banks, storage *s);

#endif
