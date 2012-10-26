/**
 * @file
 * The storage interface header
 */
#ifndef MECHANIC_STORAGE_H
#define MECHANIC_STORAGE_H

#include "Mechanic2.h"
#include "MLog.h"
#include "MTypes.h"
#include "MCommon.h"
#include "MModules.h"
#include "MTask.h"

#define STORAGE_FULL 1
#define STORAGE_DEFAULT 0

int CommitStorageLayout(module *m, pool *p);
int CommitMemoryLayout(int banks, storage *s);
void FreeMemoryLayout(int banks, storage *s);
int Storage(module *m, pool *p);
int CheckLayout(int banks, storage *s);

int CreateDataset(hid_t location, storage *s, module *m, pool *p);
int CommitDataset(hid_t h5location, storage *s, double **data);
int CommitData(hid_t h5location, int banks, storage *s);
int ReadDataset(hid_t h5location, int banks, storage *s);

int GetBanks(int allocated_banks, storage *s);

void StorageFinalize(int banks, storage *s);

#endif
