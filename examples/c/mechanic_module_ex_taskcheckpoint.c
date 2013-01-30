/**
 * Task checkpoint
 * ===============
 *
 * This example shows how to use task checkpointing (snapshots)
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_taskcheckpoint.c -o libmechanic_module_ex_taskcheckpoint.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_taskcheckpoint -x 10 -y 20 -b 5 -d 2
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 */
#include "mechanic.h"

#define MAX_SNAPSHOTS 10

/**
 * Implements Storage()
 */
int Storage(pool *p, setup *s) {
  p->task->storage[0].layout = (schema) {
    .name = "result",
    .rank = TASK_BOARD_RANK,
    .dims[0] = 1,
    .dims[1] = 1,
    .dims[2] = MAX_SNAPSHOTS,
    .sync = 1,
    .use_hdf = 1,
    .storage_type = STORAGE_BOARD,
    .datatype = H5T_NATIVE_DOUBLE
  };

  return SUCCESS;
}

/**
 * Implements TaskProcess()
 * 
 * The `TASK_CHECKPOINT` return code tells the Mechanic to save current state of the
 * task in the checkpoint buffer. The data is returned to the master node, stored in the
 * checkpoint buffer and sent back to the worker node. The data is available in storage
 * banks defined in the Storage() hook. You can use t->cid checkpoint counter to
 * distinguish whether any snapshots were performed. The snapshot data can be read in a
 * usual way. 
 *
 * To finalize the task processing, use `TASK_FINALIZE` return code.
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double buffer[1][1][MAX_SNAPSHOTS];

  if (t->cid >= MAX_SNAPSHOTS) return TASK_FINALIZE;

  // A snapshot has been performed, read it 
  if (t->cid > 0) {
    MReadData(t, "result", &buffer[0][0][0]);
  }

  // Add data to the task result
  buffer[0][0][0] = t->tid;
  buffer[0][0][t->cid+1] = t->tid + t->cid;

  // Write the data (snapshot)
  MWriteData(t, "result", &buffer[0][0][0]);

  return TASK_CHECKPOINT;
}
