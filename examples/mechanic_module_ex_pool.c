/**
 * Read/Write pool data
 * ====================
 *
 * Compilation
 * -----------
 *
 *     mpicc -fPIC -Dpic -shared -lmechanic2 -lreadconfig -lhdf5 -lhdf5_hl mechanic_module_pool.c -o libmechanic_module_pool.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic2 -p pool -x 10 -y 20
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 *
 */
#include "Mechanic2.h"

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

  GetDims(&p->storage[0], dims);
  pdata = AllocateDouble2D(&p->storage[0]);

  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      pdata[i][j] = i + j;
    }
  }
  
  WritePool(p, "pool-double-data", &pdata[0][0]);

  GetDims(&p->storage[1], dims);
  idata = AllocateInt2D(&p->storage[1]);
  
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      idata[i][j] = i + j;
    }
  }

  WritePool(p, "pool-int-data", &idata[0][0]);

  FreeDouble2D(pdata);
  FreeInt2D(idata);
  
  return SUCCESS;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **all, pool *p, setup *s) {
  double **pdata;
  int **idata;
  int i, j, dims[MAX_RANK], mstat;

  GetDims(&p->storage[0], dims);
  pdata = AllocateDouble2D(&p->storage[0]);

  Message(MESSAGE_INFO, "Reading pool 'pool-double-data'\n");
  ReadPool(p, "pool-double-data", &pdata[0][0]);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      printf("%2f ", pdata[i][j]);
    }
    printf("\n");
  }
  

  GetDims(&p->storage[1], dims);
  idata = AllocateInt2D(&p->storage[1]);
  
  Message(MESSAGE_INFO, "Reading pool 'pool-int-data'\n");
  ReadPool(p, "pool-int-data", &idata[0][0]);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      printf("%2d ", idata[i][j]);
    }
    printf("\n");
  }


  FreeDouble2D(pdata);
  FreeInt2D(idata);
  
  return POOL_FINALIZE;
}

