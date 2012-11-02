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
 *    mpirun -np 4 mechanic2 -p dim -x 10 -y 20
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
#define DIM2 5
#define DIM3 6

/**
 * Implements Storage()
 */
int Storage(pool *p, setup *s) {

  /**
   * 3D integer dataset
   */
  p->storage[0].layout = (schema) {
    .path = "3d-integer-datatype-group",
    .rank = 3,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .dim[2] = DIM2,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  /**
   * 4D double dataset
   */
  p->storage[1].layout = (schema) {
    .path = "4d-double-datatype-group",
    .rank = 4,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .dim[2] = DIM2,
    .dim[3] = DIM3,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  /**
   * 2D float dataset
   */
  p->storage[2].layout = (schema) {
    .path = "2d-float-datatype-group",
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_FLOAT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  /**
   * 3D double dataset of type STORAGE_BOARD
   */
  p->task->storage[0].layout = (schema) {
    .path = "3d-double-datatype-board",
    .rank = 3,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .dim[2] = DIM2,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };

  /**
   * 2D integer dataset of type STORAGE_BOARD
   */
  p->task->storage[1].layout = (schema) {
    .path = "2d-integer-datatype-board",
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };

  /**
   * 3D integer dataset of type STORAGE_GROUP
   */
  p->task->storage[2].layout = (schema) {
    .path = "3d-integer-datatype-group",
    .rank = 3,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .dim[2] = DIM2,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  /**
   * 2D integer dataset of type STORAGE_PM3D
   */
  p->task->storage[3].layout = (schema) {
    .path = "2d-integer-datatype-pm3d",
    .rank = 2,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
  };

  /**
   * 3D integer dataset of type STORAGE_LIST
   */
  p->task->storage[4].layout = (schema) {
    .path = "3d-integer-datatype-list",
    .rank = 3,
    .dim[0] = DIM0,
    .dim[1] = DIM1,
    .dim[2] = DIM2,
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
  int ***data;
  float **floa;
  double buff[DIM0][DIM1][DIM2][DIM3];
  int i,j,k,l;
  int dims[MAX_RANK];

  /**
   * Allocate and fill the 3D integer dataset
   */
  GetDims(&current->storage[0], dims);
  data = AllocateInt3D(&current->storage[0]);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      for (k = 0; k < dims[2]; k++) {
        data[i][j][k] = i + j + k;
      }
    }
  }

  WriteData(&(current->storage[0]), &data[0][0][0]);

  /**
   * Fill the 4D double dataset
   */
  for (i = 0; i < DIM0; i++) {
    for (j = 0; j < DIM1; j++) {
      for (k = 0; k < DIM2; k++) {
        for (l = 0; l < DIM3; l++) {
          buff[i][j][k][l] = i + j + k + l;
        }
      }
    }
  }

  WriteData(&(current->storage[1]), buff);

  /**
   * Allocate and fill the 2D float dataset
   */
  GetDims(&current->storage[2], dims);
  floa = AllocateFloat2D(&current->storage[2]);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      floa[i][j] = 80.23 + i + j;
    }
  }

  WriteData(&(current->storage[2]), &floa[0][0]);

  /**
   * Release the resources
   */
  FreeInt3D(data);
  FreeFloat2D(floa);

  return SUCCESS;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **allpools, pool *current, setup *s) {
  int i,j,k;
  int dims[MAX_RANK];
  int **idata, ***tp;
  double ***data;

  Message(MESSAGE_OUTPUT, "\n3D Integer dataset of STORAGE_GROUP (XY slice-0)\n\n");
  
  tp = AllocateInt3D(&current->tasks[current->pool_size-2]->storage[2]);
  ReadData(&current->tasks[current->pool_size-2]->storage[2], &tp[0][0][0]);
  GetDims(&current->tasks[current->pool_size-2]->storage[2], dims);

  for (i = 0; i < dims[0]; i++) {
    printf("\t");
    for (j = 0; j < dims[1]; j++) {
      printf("%2d ", tp[i][j][0]);
    }
    printf("\n");
  }

  Message(MESSAGE_OUTPUT, "\n3D Integer dataset of STORAGE_GROUP (XZ slice-0)\n\n");

  for (i = 0; i < dims[0]; i++) {
    printf("\t");
    for (j = 0; j < dims[2]; j++) {
      printf("%2d ", tp[i][0][j]);
    }
    printf("\n");
  }
  
  Message(MESSAGE_OUTPUT, "\n3D Integer dataset of STORAGE_GROUP (YZ slice-0)\n\n");
  
  for (i = 0; i < dims[1]; i++) {
    printf("\t");
    for (j = 0; j < dims[2]; j++) {
      printf("%2d ", tp[0][i][j]);
    }
    printf("\n");
  }
  
  Message(MESSAGE_OUTPUT, "\n3D Integer dataset of STORAGE_GROUP (XY slice-0)\n\n");

  ReadData(&current->tasks[current->pool_size - 1]->storage[2], &tp[0][0][0]);
  
  for (i = 0; i < dims[0]; i++) {
    printf("\t");
    for (j = 0; j < dims[1]; j++) {
      printf("%2d ", tp[i][j][0]);
    }
    printf("\n");
  }


  /* Read whole dataset from the pool->task->storage[0] */
  Message(MESSAGE_OUTPUT, "\n3D Double dataset of STORAGE_BOARD (XY slice DIM2-1)\n\n");

  GetDims(&current->task->storage[0], dims);
  data = AllocateDouble3D(&current->task->storage[0]);
  ReadData(&current->task->storage[0], &data[0][0][0]);
 
  for (i = 0; i < dims[0]; i++) {
    printf("\t");
    for (j = 0; j < dims[1]; j++) {
      printf("%5.2f ", data[i][j][dims[2]-1]);
    }
    printf("\n");
  }

  Message(MESSAGE_OUTPUT, "\n2D Integer dataset of STORAGE_BOARD (XY only)\n\n");

  GetDims(&current->task->storage[1], dims);
  idata = AllocateInt2D(&current->task->storage[1]);
  ReadData(&current->task->storage[1], &idata[0][0]);
  
  for (i = 0; i < dims[0]; i++) {
    printf("\t");
    for (j = 0; j < dims[1]; j++) {
      printf("%4d ", idata[i][j]);
    }
    printf("\n");
  }

  printf("\n");

  FreeInt3D(tp);
  FreeDouble3D(data);
  FreeInt2D(idata);

  return POOL_FINALIZE;
}

/**
 * Implements TaskProcess()
 *
 * We use here dynamic allocation of buffer data with help of Allocate functions provided
 * by the Mechanic2.
 */
int TaskProcess(pool *p, task *t, setup *s) {
  int **idata;
  int ***cdata;
  double ***ddata;
  int i,j,k;
  int dims[MAX_RANK];

  /**
   * Allocate and fill the 3D double-type data buffer for t->storage[0]
   * (see p->task->storage[0].layout in Storage())
   * 
   * Since this is STORAGE_BOARD-type storage, the data will be stored in one 
   * dataset with offsets calculated automatically. Each task will return dims-size
   * datablock.
   */
  GetDims(&t->storage[0], dims);
  ddata = AllocateDouble3D(&t->storage[0]);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      for (k = 0; k < dims[2]; k++) {
        ddata[i][j][k] = t->tid+k;
      }
    }
  }
  
  WriteData(&t->storage[0], &ddata[0][0][0]);

  /**
   * Allocate and fill the 2D integer-type data for t->storage[1]
   * (see p->task->storage[1].layout in Storage())
   *
   * This is STORAGE_BOARD-type as well.
   */
  GetDims(&t->storage[1], dims);
  idata = AllocateInt2D(&t->storage[1]);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      idata[i][j] = t->tid+1;
    }
  }

  WriteData(&t->storage[1], &idata[0][0]);

  /**
   * Allocate and fill the 3D integer-type data buffer
   * (see p->task->storage[2].layout in Storage())
   * 
   * Since this is STORAGE_GROUP-type storage, the data will be stored in separated
   * datasets per each task and available through p->tasks[task-ID]->storage[2] group.
   */
  GetDims(&t->storage[2], dims);
  cdata = AllocateInt3D(&t->storage[2]);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      for (k = 0; k < dims[2]; k++) {
        cdata[i][j][k] = t->tid+1+i+j+k;
      }
    }
  }

  WriteData(&t->storage[2], &cdata[0][0][0]);

  /**
   * Fill the 2D integer-type data buffer for t->storage[3]
   * (see p->task->storage[3].layout in Storage())
   *
   * We are reusing here data for t->storage[1], since they have the same size. Of course,
   * it is possible to change the size dynamically, depending on the problem, right in
   * Storage().
   *
   * The data will be stored as STORAGE_PM3D-type dataset.
   */
  WriteData(&t->storage[3], &idata[0][0]);

  /**
   * Again, we are reusing here data for t->storage[2], this time however we are storing
   * them into STORAGE_LIST-type dataset.
   * 
   * (see p->task->storage[4].layout in Storage())
   */
  WriteData(&t->storage[4], &cdata[0][0][0]);


  /**
   * Release the resources
   */
  FreeDouble3D(ddata);
  FreeInt3D(cdata);
  FreeInt2D(idata);

  return SUCCESS;
}

