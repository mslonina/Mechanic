/**
 * @file
 * Common functions and defines
 */
#ifndef MECHANIC_COMMON_H
#define MECHANIC_COMMON_H

#include "mechanic.h"
#include "MTypes.h"
#include "MConfig.h"

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#define POOLS_GROUP "Pools"
#define POOL_PATH "/Pools/pool-%04d"
#define LAST_GROUP "/Pools/last"
#define TASKS_GROUP "Tasks"
#define TASK_PATH "task-%04d"

char* Name(char *prefix, char *name, char *suffix, char *extension);

int Copy(char *in, char *out);
int Validate(module *m, char *filename);
int MechanicHeader(module *m, hid_t h5location);

int Allocate(storage *s, size_t size, size_t datatype); /**< Memory allocator */
void Free(storage *s); /**< Garbage cleaner */
int AllocateAttribute(attr *s, size_t size, size_t datatype); /**< Memory allocator */
void FreeAttribute(attr *s); /**< Garbage cleaner */

#endif
