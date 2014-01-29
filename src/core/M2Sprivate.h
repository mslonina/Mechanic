/**
 * @file
 * Data storage and management
 */
#ifndef MECHANIC_M2S_PRIVATE_H
#define MECHANIC_M2S_PRIVATE_H

#include "M2Cprivate.h"
#include "M2Hpublic.h"
#include "M2Spublic.h"
#include "M2Tpublic.h"
#include "M2Ppublic.h"

int CommitStorageLayout(module *m, pool *p);
int Storage(module *m, pool *p);
int CheckLayout(module *m, unsigned int banks, storage *s);
int CheckAttributeLayout(attr *a);

int CreateDataset(hid_t location, storage *s, module *m, pool *p);

unsigned int GetBanks(unsigned int allocated_banks, storage *s);
unsigned int GetFields(unsigned int allocated_fields, field *f);

#endif
