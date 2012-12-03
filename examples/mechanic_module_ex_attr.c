/**
 * Using HDF5 attributes
 * =====================
 *
 * Compilation
 * -----------
 *
 *     mpicc -fPIC -Dpic -shared -lmechanic2 -lhdf5 -lhdf5_hl mechanic_module_attr.c -o libmechanic_module_attr.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic2 -p attr -x 10 -y 20
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 *
 * Note
 * ----
 *
 * Attributes for STORAGE_GROUP-type task datasets are limited, currently each
 * /Tasks/task-ID group receives the same attributes (worker nodes does not know anything
 * about attributes, it is still work to do)
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
    .name = "pool-data",
    .rank = 2,
    .dim[0] = 4,
    .dim[1] = 5,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_INT
  };

  /* Attributes for the pool dataset 
   * 
   * We define attributes in the similar fashion as datasets. Since most of attributes are
   * small data, we will tell the Mechanic to use special storage type, H5S_SIMPLE, which
   * makes the attribute to be rank 1. The attribute name is required.
   */
  p->storage[0].attr[0].layout = (schema) {
    .name = "Global integer attribute",
    .dataspace = H5S_SCALAR,
    .datatype = H5T_NATIVE_INT
  };

  p->storage[0].attr[1].layout = (schema) {
    .name = "Global double attribute",
    .dataspace = H5S_SCALAR,
    .datatype = H5T_NATIVE_DOUBLE
  };

  /* First dataset */
  p->task->storage[0].layout = (schema) {
    .name = "input",
    .rank = TASK_BOARD_RANK,
    .dim[0] = 1,
    .dim[1] = 3,
    .dim[2] = 1,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
    .datatype = H5T_NATIVE_DOUBLE
  };

  /* Attributes for the first dataset */
  p->task->storage[0].attr[0].layout = (schema) {
    .name = "Sample integer attribute",
    .dataspace = H5S_SCALAR,
    .datatype = H5T_NATIVE_INT
  };

  p->task->storage[0].attr[1].layout = (schema) {
    .name = "Sample double attribute",
    .dataspace = H5S_SCALAR,
    .datatype = H5T_NATIVE_DOUBLE
  };

  /* The result dataset */
  p->task->storage[1].layout = (schema) {
    .name = "result",
    .rank = TASK_BOARD_RANK,
    .dim[0] = 1,
    .dim[1] = 3,
    .dim[2] = 1,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
    .datatype = H5T_NATIVE_DOUBLE
  };
  
  /* HDF5 attributes may also have some dimensionality,
   * in such case, we need to tell the Mechanic to use H5S_SIMPLE dataspace,
   * and adjust the dimensions of the attribute */
  p->task->storage[1].attr[0].layout = (schema) {
    .name = "Some special attribute",
    .dataspace = H5S_SIMPLE,
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 4,
    .datatype = H5T_NATIVE_DOUBLE
  };
  
  /* The group-type dataset */
  p->task->storage[2].layout = (schema) {
    .name = "group-result",
    .rank = TASK_BOARD_RANK,
    .dim[0] = 1,
    .dim[1] = 3,
    .dim[2] = 1,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_DOUBLE
  };

  /* Attributes for the first dataset */
  p->task->storage[2].attr[0].layout = (schema) {
    .name = "Group integer attribute",
    .dataspace = H5S_SCALAR,
    .datatype = H5T_NATIVE_INT
  };

  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 *
 * We can prepare some attributes here. The attributes are broadcasted among with pool
 * data, so you may use them during the TaskProcess().
 */
int PoolPrepare(pool *all, pool *p, setup *s) {
  int iattr;
  double dattr;

  iattr = 34;
  dattr = 45.67;

  // Using direct attribute interface
  WriteAttr(&p->storage[0].attr[0], &iattr);
  WriteAttr(&p->storage[0].attr[1], &dattr);

  return SUCCESS;
}

/**
 * Implements TaskProcess()
 *
 * Here we read some pool attributes, and use them to prepare the task result
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double buffer_one[1][3];
  int iattr;
  double dattr;

  // Read pool attributes
  // Using direct attribute interface
  ReadAttr(&p->storage[0].attr[0], &iattr);
  ReadAttr(&p->storage[0].attr[1], &dattr);

  //printf("Pool attributes: %d %f\n", iattr, dattr);

  // The vertical position of the pixel
  buffer_one[0][0] = iattr + t->location[0];

  // The horizontal position of the pixel
  buffer_one[0][1] = dattr + t->location[1];

  // The state of the system
  buffer_one[0][2] = t->tid;

  WriteData(&t->storage[1], buffer_one);
  
  return SUCCESS;
}

/**
 * Implements PoolProcess()
 *
 * The attributes are attached to the dataset after all tasks are processed, and after the
 * PoolProcess() hook is invoked. It is the best place to adjust data for an attribute by
 * using WriteAttr() function.
 */
int PoolProcess(pool *all, pool *p, setup *s) {
  double dattr;
  double t_attr;
  double s_attr[1][4];
  int iattr;
  int mstat;

  iattr = 197;
  dattr = 12345.6789;
  s_attr[0][0] = 123.0;
  s_attr[0][1] = 223.0;
  s_attr[0][2] = 323.0;
  s_attr[0][3] = 423.0;

  /* Write some attributes */
  mstat = WriteTaskAttr(p->task, "input", "Sample integer attribute", &iattr);
  mstat = WriteTaskAttr(p->task, "input", "Sample double attribute", &dattr);

  /* You may use explicit interface, instead: */
  //mstat = WriteAttr(&p->task->storage[0].attr[0], &attr);
  //mstat = WriteAttr(&p->task->storage[0].attr[1], &s_attr);

  mstat = ReadTaskAttr(p->task, "input", "Sample double attribute", &t_attr);
  
  /* Or using explicit interface: */
  //mstat = ReadAttr(&p->task->storage[0].attr[1], &t_attr);
  
  Message(MESSAGE_OUTPUT, "Attribute = %f\n", t_attr);

  return POOL_FINALIZE;
}

