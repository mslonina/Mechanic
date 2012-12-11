/**
 * Use different datatypes
 * =======================
 *
 * In this example we show usage of different (native) datatypes
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_datatypes.c -o libmechanic_ex_module_datatypes.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_datatypes -x 10 -y 20
 *
 * Listing the contents of the data file
 * -------------------------------------
 *
 *    h5ls -r mechanic-master-00.h5
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 */

#include "mechanic.h"

#define DIM0 3
#define DIM1 4

#define PSET0 "integer-datatype"
#define PSET1 "double-datatype"
#define PSET2 "float-datatype"

#define TSET0 "double-datatype"
#define TSET1 "integer-datatype"
#define TSET2 "integer-datatype-group"
#define TSET3 "TSET3"
#define TSET4 "TSET4"

/**
 * Implements Storage()
 */
int Storage(pool *p, setup *s) {

  p->storage[0].layout = (schema) {
    .name = PSET0,
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->storage[1].layout = (schema) {
    .name = PSET1,
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->storage[2].layout = (schema) {
    .name = PSET2,
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_FLOAT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->task->storage[0].layout = (schema) {
    .name = TSET0,
    .rank = TASK_BOARD_RANK,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .dim[2] = 1,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };

  p->task->storage[1].layout = (schema) {
    .name = TSET1,
    .rank = TASK_BOARD_RANK,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .dim[2] = 1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };

  p->task->storage[2].layout = (schema) {
    .name = TSET2,
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->task->storage[3].layout = (schema) {
    .name = TSET3,
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
  };

  p->task->storage[4].layout = (schema) {
    .name = TSET4,
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_LIST,
  };

  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 */
int PoolPrepare(pool **allpools, pool *current, setup *s) {
  int data[DIM0][DIM1];
  double buff[DIM0][DIM1];
  float floa[DIM0][DIM1];
  int i,j;

  for (i = 0; i < DIM0; i++) {
    for (j = 0; j < DIM1; j++) {
      data[i][j] = i + j;
      buff[i][j] = 47.13 + i + j;
      floa[i][j] = 80.23 + i + j;
    }
  }

  MWriteData(current, PSET0, &data[0][0]);
  MWriteData(current, PSET1, &buff[0][0]);
  MWriteData(current, PSET2, &floa[0][0]);

  return SUCCESS;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **allpools, pool *current, setup *s) {
  int i, j, k, l;
  int **ibuff, ***cbuff;
  double ***dbuff;
  int dims[MAX_RANK];
  
  /**
   * Read the dataset inside the Tasks/task-[i] group
   */
  MAllocate2(current->tasks[current->pool_size - 1], TSET2, ibuff, int);
  MReadData(current->tasks[current->pool_size - 1], TSET2, &ibuff[0][0]);

  MGetDims(current->tasks[current->pool_size - 1], TSET2, dims);

  Message(MESSAGE_OUTPUT, "Integer dataset of STORAGE_GROUP\n");
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      Message(MESSAGE_OUTPUT, "%2d ", ibuff[i][j]);
    }
    printf("\n");
  }

  free(ibuff);

  /**
   * Read the p->task->storage[0] dataset
   *
   * This is the STORAGE_BOARD dataset, which means, that the buffer must have the size
   * dims[0] = p->task->storage[0].layout.dim[0] * p->board->layout.dim[0]
   * dims[1] = p->task->storage[0].layout.dim[1] * p->board->layout.dim[1]
   */
  MAllocate3(current->task, TSET0, dbuff, double);
  MReadData(current->task, TSET0, &dbuff[0][0][0]);

  MGetDims(current->task, TSET0, dims);

  Message(MESSAGE_OUTPUT, "\nDouble dataset of STORAGE_BOARD\n");
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      Message(MESSAGE_OUTPUT, "%5.2f ", dbuff[i][j][0]);
    }
    printf("\n");
  }

  free(dbuff);

  /**
   * Read the p->task->storage[1] dataset
   *
   * This is the STORAGE_BOARD dataset, which means, that the buffer must have the size
   * dims[0] = p->task->storage[1].layout.dim[0] * p->board->layout.dim[0]
   * dims[1] = p->task->storage[1].layout.dim[1] * p->board->layout.dim[1]
   */
  MAllocate3(current->task, TSET1, cbuff, int);
  MReadData(current->task, TSET1, &cbuff[0][0][0]);

  MGetDims(current->task, TSET1, dims);

  Message(MESSAGE_OUTPUT, "\nInteger dataset of STORAGE_BOARD\n");
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      Message(MESSAGE_OUTPUT, "%2d ", cbuff[i][j][0]);
    }
    printf("\n");
  }

  free(cbuff);

  /**
   * Read the p->task->storage[3] dataset
   *
   * This is the STORAGE_PM3D dataset, which means, that the buffer must have the size
   * dims[0] = p->task->storage[3].layout.dim[0] * p->pool_size
   * dims[1] = p->task->storage[3].layout.dim[1] 
   */
  MAllocate2(current->task, TSET3, ibuff, int);
  MReadData(current->task, TSET3, &ibuff[0][0]);
  
  MGetDims(current->task, TSET3, dims);

  Message(MESSAGE_OUTPUT, "\nInteger dataset of STORAGE_PM3D\n");
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      Message(MESSAGE_OUTPUT, "%2d ", ibuff[i][j]);
    }
    printf("\n");
  }

  free(ibuff);

  /**
   * Read the p->task->storage[4] dataset
   *
   * This is the STORAGE_LIST dataset, which means, that the buffer must have the size
   * dims[0] = p->task->storage[3].layout.dim[0] * p->pool_size
   * dims[1] = p->task->storage[3].layout.dim[1] 
   */
  MAllocate2(current->task, TSET4, ibuff, int);
  MReadData(current->task, TSET4, &ibuff[0][0]);
  
  MGetDims(current->task, TSET4, dims);

  Message(MESSAGE_OUTPUT, "\nInteger dataset of STORAGE_LIST\n");
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      Message(MESSAGE_OUTPUT, "%2d ", ibuff[i][j]);
    }
    printf("\n");
  }

  free(ibuff);

  return POOL_FINALIZE;
}

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double data[DIM0][DIM1][1];
  int idata[DIM0][DIM1];
  int cdata[DIM0][DIM1][1];
  int i,j;

  for (i = 0; i < DIM0; i++) {
    for (j = 0; j < DIM1; j++) {
      data[i][j][0] = t->tid+1.1;
      idata[i][j] = t->tid+1;
      cdata[i][j][0] = t->tid+1;
    }
  }

  MWriteData(t, TSET0, &data[0][0][0]);
  MWriteData(t, TSET1, &cdata[0][0][0]);
  MWriteData(t, TSET2, &idata[0][0]);
  MWriteData(t, TSET3, &idata[0][0]);
  MWriteData(t, TSET4, &idata[0][0]);

  return SUCCESS;
}

