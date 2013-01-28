/**
 * Use different storage dimensions
 * ================================
 *
 * In this example we show usage of different storage dimensions
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_dim.c -o libmechanic_module_ex_dim.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_dim -x 10 -y 20
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
#define DIM2 5
#define DIM3 6

#define PSET0 "3d-integer-datatype-group"
#define PSET1 "4d-double-datatype-group"
#define PSET2 "2d-float-datatype-group"
#define TSET0 "3d-double-datatype-board"
#define TSET1 "2d-integer-datatype-board"
#define TSET2 "3d-integer-datatype-group"
#define TSET3 "2d-integer-datatype-pm3d"
#define TSET4 "3d-integer-datatype-list"
#define TSET5 "4d-result"

/**
 * Implements Storage()
 */
int Storage(pool *p, setup *s) {

  /**
   * 3D integer dataset
   */
  p->storage[0].layout = (schema) {
    .name = PSET0,
    .rank = 3,
    .dims[0] = DIM0,
    .dims[1] = DIM1,
    .dims[2] = DIM2,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  /**
   * 4D double dataset
   */
  p->storage[1].layout = (schema) {
    .name = PSET1,
    .rank = 4,
    .dims[0] = DIM0,
    .dims[1] = DIM1,
    .dims[2] = DIM2,
    .dims[3] = DIM3,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  /**
   * 2D float dataset
   */
  p->storage[2].layout = (schema) {
    .name = PSET2,
    .rank = 2,
    .dims[0] = DIM0,
    .dims[1] = DIM1,
    .datatype = H5T_NATIVE_FLOAT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  /**
   * 3D double dataset of type STORAGE_BOARD
   */
  p->task->storage[0].layout = (schema) {
    .name = TSET0,
    .rank = TASK_BOARD_RANK,
    .dims[0] = DIM0,
    .dims[1] = DIM1,
    .dims[2] = DIM2,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };

  /**
   * 2D integer dataset of type STORAGE_BOARD
   */
  p->task->storage[1].layout = (schema) {
    .name = TSET1,
    .rank = TASK_BOARD_RANK,
    .dims[0] = DIM0,
    .dims[1] = DIM1,
    .dims[2] = 1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };

  /**
   * 3D integer dataset of type STORAGE_GROUP
   */
  p->task->storage[2].layout = (schema) {
    .name = TSET2,
    .rank = 3,
    .dims[0] = DIM0,
    .dims[1] = DIM1,
    .dims[2] = DIM2,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
  };

  /**
   * 2D integer dataset of type STORAGE_PM3D
   */
  p->task->storage[3].layout = (schema) {
    .name = TSET3,
    .rank = 2,
    .dims[0] = DIM0,
    .dims[1] = DIM1,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
  };

  /**
   * 3D integer dataset of type STORAGE_LIST
   */
  p->task->storage[4].layout = (schema) {
    .name = TSET4,
    .rank = 3,
    .dims[0] = DIM0,
    .dims[1] = DIM1,
    .dims[2] = DIM2,
    .datatype = H5T_NATIVE_INT,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_LIST,
  };

  /**
   * 4D double dataset of type STORAGE_BOARD
   */
  p->task->storage[5].layout = (schema) {
    .name = TSET5,
    .rank = 4,
    .dims[0] = DIM0,
    .dims[1] = DIM1,
    .dims[2] = DIM2,
    .dims[3] = DIM3,
    .datatype = H5T_NATIVE_DOUBLE,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
  };

  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 */
int PoolPrepare(pool **allpools, pool *current, setup *s) {
  int ***data;
  float **floa;
  double ****buff;
  int i,j,k,l;
  int dims[MAX_RANK];

  /**
   * Allocate and fill the 3D integer dataset
   */
  MAllocate3(current, PSET0, data, int);
  MGetDims(current, PSET0, dims);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      for (k = 0; k < dims[2]; k++) {
        data[i][j][k] = i + j + k;
      }
    }
  }

  MWriteData(current, PSET0, &data[0][0][0]);

  /**
   * Allocate and fill the 4D double dataset
   */
  MAllocate4(current, PSET1, buff, double);
  MGetDims(current, PSET1, dims);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      for (k = 0; k < dims[2]; k++) {
        for (l = 0; l < dims[3]; l++) {
          buff[i][j][k][l] = i+j+k+l;
        }
      }
    }
  }

  MWriteData(current, PSET1, &buff[0][0][0][0]);

  /**
   * Allocate and fill the 2D float dataset
   */
  MAllocate2(current, PSET2, floa, float);
  MGetDims(current, PSET2, dims);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      floa[i][j] = 80.23 + i + j;
    }
  }

  MWriteData(current, PSET2, &floa[0][0]);

  /**
   * Release the resources
   */
  free(data);
  free(floa);
  free(buff);

  return SUCCESS;
}

/**
 * Implements PoolProcess()
 */
int PoolProcess(pool **allpools, pool *current, setup *s) {
  int i,j,k;
  int dims[MAX_RANK];
  int ***idata, ***tp;
  double ***data;
  double ****four;

  Message(MESSAGE_OUTPUT, "\n3D Integer dataset of STORAGE_GROUP (XY slice-0)\n\n");
  
  MAllocate3(current->tasks[0], TSET2, tp, int);

  MReadData(current->tasks[0], TSET2, &tp[0][0][0]);
  MGetDims(current->tasks[0], TSET2, dims);

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

  MReadData(current->tasks[current->pool_size - 1], TSET2, &tp[0][0][0]);
  
  for (i = 0; i < dims[0]; i++) {
    printf("\t");
    for (j = 0; j < dims[1]; j++) {
      printf("%2d ", tp[i][j][0]);
    }
    printf("\n");
  }

  /* Read whole dataset from the pool->task->storage[0] */
  Message(MESSAGE_OUTPUT, "\n3D Double dataset of STORAGE_BOARD (XY slice DIM2-1)\n\n");

  MAllocate3(current->task, TSET0, data, double);
  
  MGetDims(current->task, TSET0, dims);
  MReadData(current->task, TSET0, &data[0][0][0]);
 
  for (i = 0; i < dims[0]; i++) {
    printf("\t");
    for (j = 0; j < dims[1]; j++) {
      printf("%5.2f ", data[i][j][0]);
    }
    printf("\n");
  }
  
  printf("\n");

  for (i = 0; i < dims[0]; i++) {
    printf("\t");
    for (j = 0; j < dims[1]; j++) {
      printf("%5.2f ", data[i][j][5]);
    }
    printf("\n");
  }

  Message(MESSAGE_OUTPUT, "\n2D Integer dataset of STORAGE_BOARD (XY only)\n\n");

  MGetDims(current->task, TSET1, dims);
  MAllocate3(current->task, TSET1, idata, int);
  MReadData(current->task, TSET1, &idata[0][0][0]);
  
  for (i = 0; i < dims[0]; i++) {
    printf("\t");
    for (j = 0; j < dims[1]; j++) {
      printf("%4d ", idata[i][j][0]);
    }
    printf("\n");
  }
  
  printf("\n");
  
  for (i = 0; i < dims[0]; i++) {
    printf("\t");
    for (j = 0; j < dims[1]; j++) {
      printf("%4d ", idata[i][j][1]);
    }
    printf("\n");
  }

  printf("\n");

  Message(MESSAGE_OUTPUT, "\n4D double result dataset\n\n");
  MGetDims(current->task, TSET5, dims);
  MAllocate4(current->task, TSET5, four, double);
  MReadData(current->task, TSET5, &four[0][0][0][0]);

  for (k = 0; k < dims[2]; k++) {
    printf("\t4D depth = %d\n", k);
    for (i = 0; i < dims[0]; i++) {
      printf("\t");
      for (j = 0; j < dims[1]; j++) {
        printf("%5.2f ", four[i][j][k][2]);
      }
      printf("\n");
    }
    printf("\n");
  }

  free(tp); free(data); free(idata); free(four);

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
  int ***bdata;
  int ***cdata;
  double ***ddata;
  double ****four;
  int i,j,k,l;
  int dims[MAX_RANK];

  /**
   * Allocate and fill the 3D double-type data buffer for t->storage[0]
   * (see p->task->storage[0].layout in Storage())
   * 
   * Since this is STORAGE_BOARD-type storage, the data will be stored in one 
   * dataset with offsets calculated automatically. Each task will return dims-size
   * datablock.
   */
  MGetDims(t, TSET0, dims);
  MAllocate3(t, TSET0, ddata, double);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      for (k = 0; k < dims[2]; k++) {
        ddata[i][j][k] = t->tid+k;
      }
    }
  }
  
  MWriteData(t, TSET0, &ddata[0][0][0]);

  /**
   * Allocate and fill the 3D (2D and 1) integer-type data for t->storage[1]
   * (see p->task->storage[1].layout in Storage())
   *
   * This is STORAGE_BOARD-type as well.
   */
  MGetDims(t, TSET1, dims);
  MAllocate3(t, TSET1, bdata, int);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      bdata[i][j][0] = t->tid+1;
    }
  }

  MWriteData(t, TSET1, &bdata[0][0][0]);

  /**
   * Allocate and fill the 3D integer-type data buffer
   * (see p->task->storage[2].layout in Storage())
   * 
   * Since this is STORAGE_GROUP-type storage, the data will be stored in separated
   * datasets per each task and available through p->tasks[task-ID]->storage[2] group.
   */
  MGetDims(t, TSET2, dims);
  MAllocate3(t, TSET2, cdata, int);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      for (k = 0; k < dims[2]; k++) {
        cdata[i][j][k] = t->tid+1;
      }
    }
  }

  MWriteData(t, TSET2, &cdata[0][0][0]);

  /**
   * Allocate and fill the 2D integer-type data buffer for t->storage[3]
   * (see p->task->storage[3].layout in Storage())
   *
   * The data will be stored as STORAGE_PM3D-type dataset.
   */
  MGetDims(t, TSET3, dims);
  MAllocate2(t, TSET3, idata, int);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      idata[i][j] = t->tid+1;
    }
  }

  MWriteData(t, TSET3, &idata[0][0]);

  /**
   * We are reusing here data for t->storage[2], this time however we are storing
   * them into STORAGE_LIST-type dataset.
   * 
   * (see p->task->storage[4].layout in Storage())
   */
  MWriteData(t, TSET4, &cdata[0][0][0]);

  /**
   * Now, here is the real fun
   * We would like to return 4D double dataset of STORAGE_BOARD for the 3D TASK BOARD
   *
   * This is real example of what scientific things Mechanic can do
   */
  MGetDims(t, TSET5, dims);
  MAllocate4(t, TSET5, four, double);
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      for (k = 0; k < dims[2]; k++) {
        four[i][j][k][0] = t->tid;
        four[i][j][k][1] = t->location[0];
        four[i][j][k][2] = t->location[1];
        four[i][j][k][3] = t->location[2];
        four[i][j][k][4] = t->tid + 1.1;
        four[i][j][k][5] = t->tid + 2.1;
      }
    }
  }

  MWriteData(t, TSET5, &four[0][0][0][0]);

  /**
   * Release the resources
   */
  free(ddata); free(idata); free(cdata); free(four);

  return TASK_FINALIZE;
}

