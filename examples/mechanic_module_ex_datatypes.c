/**
 * Use different datatypes
 * =======================
 *
 * In this example we show usage of different (native) datatypes
 *
 * Compilation
 * -----------
 *
 *     mpicc -fPIC -Dpic -shared -lmechanic2 mechanic_module_datatypes.c -o libmechanic_module_datatypes.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic2 -p datatypes -x 10 -y 20
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

#include "Mechanic2.h"

#define DIM0 3
#define DIM1 4

/**
 * Implements Storage()
 */
int Storage(pool *p, setup *s) {

  p->storage[0].layout = (schema) {
    .path = "integer-datatype",
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->storage[1].layout = (schema) {
    .path = "double-datatype",
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->storage[2].layout = (schema) {
    .path = "float-datatype",
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_FLOAT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->task->storage[0].layout = (schema) {
    .path = "double-datatype",
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };

  p->task->storage[1].layout = (schema) {
    .path = "integer-datatype",
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };

  p->task->storage[2].layout = (schema) {
    .path = "integer-datatype",
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  p->task->storage[3].layout = (schema) {
    .path = "integer-datatype-pm3d",
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
  };

  p->task->storage[4].layout = (schema) {
    .path = "integer-datatype-list",
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

  WriteData(&(current->storage[0]), data);
  WriteData(&(current->storage[1]), buff);
  WriteData(&(current->storage[2]), floa);

  return SUCCESS;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **allpools, pool *current, setup *s) {
  int ibuff[DIM0][DIM1], i, j, k, l;
  int idataset[75][4];
  double odataset[15][20];
  int **ibuff_two;
  double **dbuff;
  int dims[2];

  /* Read dataset inside the Tasks/task-[i] group, we already know the buffer size */
  ReadData(&current->tasks[current->pool_size - 1]->storage[2], ibuff);

  Message(MESSAGE_OUTPUT, "Integer dataset of STORAGE_GROUP\n");
  for (i = 0; i < DIM0; i++) {
    for (j = 0; j < DIM1; j++) {
      Message(MESSAGE_OUTPUT, "%d ", ibuff[i][j]);
    }
    printf("\n");
  }

  /**
   * Read the p->task->storage[0] dataset
   *
   * This is the STORAGE_BOARD dataset, which means, that the buffer must have the size
   * dims[0] = p->task->storage[0].layout.dim[0] * p->board->layout.dim[0]
   * dims[1] = p->task->storage[0].layout.dim[1] * p->board->layout.dim[1]
   */
  dims[0] = current->task->storage[0].layout.dim[0] * current->board->layout.dim[0];
  dims[1] = current->task->storage[0].layout.dim[1] * current->board->layout.dim[1];
  dbuff = AllocateDouble2D(2, dims);

  //memcpy(odataset, current->task->storage[0].memory, dims[0]*dims[1]*current->task->storage[0].layout.datatype_size);
  memcpy(&dbuff[0][0], current->task->storage[0].memory, dims[0]*dims[1]*current->task->storage[0].layout.datatype_size);

  Message(MESSAGE_OUTPUT, "\nDouble dataset of STORAGE_BOARD\n");
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      Message(MESSAGE_OUTPUT, "%5.2f ", dbuff[i][j]);
    }
    printf("\n");
  }

  FreeBuffer(dbuff);

  /**
   * Read the p->task->storage[1] dataset
   *
   * This is the STORAGE_BOARD dataset, which means, that the buffer must have the size
   * dims[0] = p->task->storage[1].layout.dim[0] * p->board->layout.dim[0]
   * dims[1] = p->task->storage[1].layout.dim[1] * p->board->layout.dim[1]
   */
  dims[0] = current->task->storage[1].layout.dim[0] * current->board->layout.dim[0];
  dims[1] = current->task->storage[1].layout.dim[1] * current->board->layout.dim[1];
  ibuff_two = AllocateInt2D(2, dims);

  memcpy(&ibuff_two[0][0], current->task->storage[1].memory, dims[0]*dims[1]*current->task->storage[1].layout.datatype_size);

  Message(MESSAGE_OUTPUT, "\nInteger dataset of STORAGE_BOARD\n");
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      Message(MESSAGE_OUTPUT, "%02d ", ibuff_two[i][j]);
    }
    printf("\n");
  }

  FreeIntBuffer(ibuff_two);

  /**
   * Read the p->task->storage[3] dataset
   *
   * This is the STORAGE_PM3D dataset, which means, that the buffer must have the size
   * dims[0] = p->task->storage[3].layout.dim[0] * p->pool_size
   * dims[1] = p->task->storage[3].layout.dim[1] 
   */
  dims[0] = current->task->storage[3].layout.dim[0] * current->pool_size;
  dims[1] = current->task->storage[3].layout.dim[1];
  ibuff_two = AllocateInt2D(2, dims);

  memcpy(&ibuff_two[0][0], current->task->storage[3].memory, dims[0]*dims[1]*current->task->storage[3].layout.datatype_size);

  Message(MESSAGE_OUTPUT, "\nInteger dataset of STORAGE_PM3D\n");
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      Message(MESSAGE_OUTPUT, "%02d ", ibuff_two[i][j]);
    }
    printf("\n");
  }

  FreeIntBuffer(ibuff_two);

  /**
   * Read the p->task->storage[4] dataset
   *
   * This is the STORAGE_LIST dataset, which means, that the buffer must have the size
   * dims[0] = p->task->storage[3].layout.dim[0] * p->pool_size
   * dims[1] = p->task->storage[3].layout.dim[1] 
   */
  dims[0] = current->task->storage[4].layout.dim[0] * current->pool_size;
  dims[1] = current->task->storage[4].layout.dim[1];
  ibuff_two = AllocateInt2D(2, dims);

  memcpy(&ibuff_two[0][0], current->task->storage[4].memory, dims[0]*dims[1]*current->task->storage[4].layout.datatype_size);

  Message(MESSAGE_OUTPUT, "\nInteger dataset of STORAGE_LIST\n");
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      Message(MESSAGE_OUTPUT, "%02d ", ibuff_two[i][j]);
    }
    printf("\n");
  }

  FreeIntBuffer(ibuff_two);

  return POOL_FINALIZE;
}

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double data[DIM0][DIM1];
  int idata[DIM0][DIM1];
  int i,j;

  for (i = 0; i < DIM0; i++) {
    for (j = 0; j < DIM1; j++) {
      data[i][j] = t->tid+1.1;
      idata[i][j] = t->tid+1;
    }
  }

  WriteData(&t->storage[0], data);
  WriteData(&t->storage[1], idata);
  WriteData(&t->storage[2], idata);
  WriteData(&t->storage[3], idata);
  WriteData(&t->storage[4], idata);

  return SUCCESS;
}

