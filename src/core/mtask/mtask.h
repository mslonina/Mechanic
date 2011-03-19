#ifndef MECHANIC_TASK

#include "mechanic_public.h"
#include "mechanic_internals.h"

typedef struct {
  int number;
  int coordinates[MECHANIC_HDF5_MAX_DIM];
  int status;
} mtask;

mtask* mtask_init();
int mtask_get_status(mtask *task);
int mtask_set_status(mtask *task, int status);
void mtask_cleanup(mtask *task);

#endif
