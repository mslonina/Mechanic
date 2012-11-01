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
#define DIM2 5
#define DIM3 6

/**
 * Implements Storage()
 */
int Storage(pool *p, setup *s) {

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
  int data[DIM0][DIM1][DIM2];
  double buff[DIM0][DIM1][DIM2][DIM3];
  float floa[DIM0][DIM1];
  int i,j,k,l;

  // 2d float
  for (i = 0; i < DIM0; i++) {
    for (j = 0; j < DIM1; j++) {
      floa[i][j] = 80.23 + i + j;
    }
  }

  // 3d integer
  for (i = 0; i < DIM0; i++) {
    for (j = 0; j < DIM1; j++) {
      for (k = 0; k < DIM2; k++) {
        data[i][j][k] = i + j + k;
      }
    }
  }

  // 4d double
  for (i = 0; i < DIM0; i++) {
    for (j = 0; j < DIM1; j++) {
      for (k = 0; k < DIM2; k++) {
        for (l = 0; l < DIM3; l++) {
          buff[i][j][k][l] = i + j + k + l;
        }
      }
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
  int three[DIM0][DIM1][DIM2];
  int idata[5*DIM0][5*DIM1];
  double data[5*DIM0][5*DIM1][DIM2];
  int i,j,k;

  /* Direct access to the memory block */
  printf("\n");
  Message(MESSAGE_RESULT, "3D Integer dataset of STORAGE_GROUP (XY slice-0)\n");
  Message(MESSAGE_RESULT, "Direct access\n");
  printf("\n");
  
  memcpy(three, current->tasks[current->pool_size-2]->storage[2].memory, DIM0*DIM1*DIM2*sizeof(int));

  for (i = 0; i < DIM0; i++) {
    printf("\t");
    for (j = 0; j < DIM1; j++) {
      printf("%2d ", three[i][j][0]);
    }
    printf("\n");
  }

  printf("\n");
  Message(MESSAGE_RESULT, "3D Integer dataset of STORAGE_GROUP (XZ slice-0)\n");
  Message(MESSAGE_RESULT, "Direct access\n");
  printf("\n");

  for (i = 0; i < DIM0; i++) {
    printf("\t");
    for (j = 0; j < DIM2; j++) {
      printf("%2d ", three[i][0][j]);
    }
    printf("\n");
  }
  
  printf("\n");
  Message(MESSAGE_RESULT, "3D Integer dataset of STORAGE_GROUP (YZ slice-0)\n");
  Message(MESSAGE_RESULT, "Direct access\n");
  printf("\n");
  
  for (i = 0; i < DIM1; i++) {
    printf("\t");
    for (j = 0; j < DIM2; j++) {
      printf("%2d ", three[0][i][j]);
    }
    printf("\n");
  }

  /* Read dataset inside the Tasks/task-[i] group, we already know the buffer size */
  printf("\n");
  Message(MESSAGE_RESULT, "3D Integer dataset of STORAGE_GROUP (XY slice-0)\n");
  Message(MESSAGE_RESULT, "Usage of ReadData()\n");
  printf("\n");

  ReadData(&current->tasks[current->pool_size - 1]->storage[2], three);
  
  for (i = 0; i < DIM0; i++) {
    printf("\t");
    for (j = 0; j < DIM1; j++) {
      printf("%2d ", three[i][j][0]);
    }
    printf("\n");
  }

  /* Read whole dataset from the pool->task->storage[0] */
  printf("\n");
  Message(MESSAGE_RESULT, "3D Double dataset of STORAGE_BOARD (XY slice DIM2-1)\n");
  Message(MESSAGE_RESULT, "Direct access\n");
  printf("\n");

  memcpy(data, current->task->storage[0].memory, 5*DIM0*5*DIM1*DIM2*sizeof(double));
 
  for (i = 0; i < 5*DIM0; i++) {
    printf("\t");
    for (j = 0; j < 5*DIM1; j++) {
      printf("%5.2f ", data[i][j][DIM2-1]);
    }
    printf("\n");
  }

  printf("\n");
  Message(MESSAGE_RESULT, "2D Integer dataset of STORAGE_BOARD (XY only)\n");
  Message(MESSAGE_RESULT, "Direct access\n");
  printf("\n");

  memcpy(idata, current->task->storage[1].memory, 5*DIM0*5*DIM1*sizeof(int));
  
  for (i = 0; i < 5*DIM0; i++) {
    printf("\t");
    for (j = 0; j < 5*DIM1; j++) {
      printf("%4d ", idata[i][j]);
    }
    printf("\n");
  }

  printf("\n");
  return POOL_FINALIZE;
}

/**
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double data[DIM0][DIM1];
  int idata[DIM0][DIM1];
  int cdata[DIM0][DIM1][DIM2];
  double ddata[DIM0][DIM1][DIM2];
  int i,j,k,l,z;

  for (i = 0; i < DIM0; i++) {
    for (j = 0; j < DIM1; j++) {
      data[i][j] = t->tid+1.1;
      idata[i][j] = t->tid+1;
    }
  }

  for (i = 0; i < DIM0; i++) {
    for (j = 0; j < DIM1; j++) {
      for (k = 0; k < DIM2; k++) {
        cdata[i][j][k] = t->tid+1;
        ddata[i][j][k] = t->tid+k;
      }
    }
  }

  WriteData(&t->storage[0], ddata);
  WriteData(&t->storage[1], idata);
  WriteData(&t->storage[2], cdata);
  WriteData(&t->storage[3], idata);
  WriteData(&t->storage[4], cdata);

  return SUCCESS;
}

