#ifndef MECHANIC_COMMON_H
#define MECHANIC_COMMON_H

#include "MMechanic2.h"

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#define POOLS_GROUP "Pools"
#define POOL_PATH "/Pools/pool-%04d"
#define LAST_GROUP "/Pools/last"
#define TASKS_GROUP "Tasks"
#define TASK_PATH "task-%04d"

char* Name(char *prefix, char *name, char *suffix, char *extension);
void Error(int status);
void Abort(int status);
void CheckStatus(int status);
void H5CheckStatus(hid_t status);

void Array2Vec(double *vec, double **array, int rank, int *dims);
void Vec2Array(double *vec, double **array, int rank, int *dims);

int Copy(char *in, char *out);
int Validate(char *filename);

#endif
