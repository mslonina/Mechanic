/**
 * Read/Write pool data
 * ====================
 *
 * In this example, we write and read pool data (using PoolPrepare() and PoolProcess()
 * hooks)
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_pool.c -o libmechanic_module_ex_pool.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_pool -x 10 -y 20
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 *
 */
#include "mechanic.h"

/**
 * Implements Storage()
 *
 * Each worker will return 1x3 result array. The master node will combine the worker
 * result arrays into one dataset suitable to process with Matplotlib. The final dataset
 * will be available at /Pools/pool-0000/Tasks/result.
 *
 */
int Storage(pool *p, setup *s) {
  p->storage[0].layout = (schema) {
    .name = "pool-double-data",
    .rank = 2,
    .dim[0] = 4,
    .dim[1] = 5,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_DOUBLE
  };

  p->storage[1].layout = (schema) {
    .name = "pool-int-data",
    .rank = 2,
    .dim[0] = 4,
    .dim[1] = 5,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_INT
  };

  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 */
int PoolPrepare(pool **all, pool *p, setup *s) {
  double **pdata;
  int **idata;
  int i, j, dims[MAX_RANK], mstat;

  MGetDims(p, "pool-double-data" , dims);
  
  // Direct interface:
  //GetDims(&p->storage[0], dims);

  MAllocate2(p, "pool-double-data", pdata, double);
  
  // Direct interface:
  //pdata = AllocateDouble2D(&p->storage[0]);

  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      pdata[i][j] = i + j;
    }
  }
  
  MWriteData(p, "pool-double-data", &pdata[0][0]);

  // Direct interface:
  // WriteData(&p->storage[0], &pdata[0][0]);

  MGetDims(p, "pool-int-data", dims);
  MAllocate2(p, "pool-int-data", idata, int);
  
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      idata[i][j] = i + j;
    }
  }

  MWriteData(p, "pool-int-data", &idata[0][0]);

  free(pdata);
  free(idata);
  
  return SUCCESS;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **all, pool *p, setup *s) {
  double **pdata;
  int **idata;
  int i, j, dims[MAX_RANK], mstat;

  MGetDims(p, "pool-double-data", dims);
  MAllocate2(p, "pool-double-data", pdata, double);

  Message(MESSAGE_INFO, "Reading pool 'pool-double-data'\n");
  MReadData(p, "pool-double-data", &pdata[0][0]);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      printf("%2f ", pdata[i][j]);
    }
    printf("\n");
  }

  MGetDims(p, "pool-int-data", dims);
  MAllocate2(p, "pool-int-data", idata, int);
  
  Message(MESSAGE_INFO, "Reading pool 'pool-int-data'\n");
  MReadData(p, "pool-int-data", &idata[0][0]);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      printf("%2d ", idata[i][j]);
    }
    printf("\n");
  }

  free(pdata);
  free(idata);
  
  return POOL_FINALIZE;
}

