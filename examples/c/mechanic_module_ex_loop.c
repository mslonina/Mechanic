/**
 * Using LoopPrepare()/Process() hooks
 * ===================================
 *
 * These advanced hooks might be used in case that additional MPI communication is
 * neccessary, after the PoolPrepare() in/or PoolProcess().
 *
 * The LoopPrepare() is invoked after the PoolPrepare().
 * The LoopProcess() is invoked before the PoolProcess().
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_loop.c -o libmechanic_module_ex_loop.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_loop -x 10 -y 20
 *
 */
#include "mechanic.h"

int info;

/**
 * Implements Init()
 */
int Init(init *i) {
  i->pools = 10;
  return SUCCESS;
}

/**
 * Implements LoopPrepare()
 *
 * According to the pool id we send additional message to all nodes
 */
int LoopPrepare(int mpi_size, int node, pool **all, pool *p) {
  if (p->pid == 2) {
    if (node == MASTER) info = 99;
    MPI_Bcast(&info, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
  } else {
    info = 0;
  }
  return SUCCESS;
} 

/**
 * TaskPrepare()
 * TaskProcess()
 */

/**
 * Implements LoopProcess()
 *
 * We check for additional message/setting
 */
int LoopProcess(int mpi_size, int node, pool **all, pool *p) {
  if (p->rid == 1 && info == 99) {
    Message(MESSAGE_OUTPUT, "Node %d, pool %d, reset id = %d\n", node, p->pid, p->rid); 
  }
  return SUCCESS;
} 

/**
 * Implements PoolProcess()
 *
 * Here, we reset the pool until the p->rid < 3. Then, we create new pool. If we reach 5
 * pools, we finalize the pool loop.
 */
int PoolProcess(pool **all, pool *p) {
  if (p->rid < 3) return POOL_RESET;
  if (p->pid < 5) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}

