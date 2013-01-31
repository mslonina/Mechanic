/**
 * Using NodePrepare() and NodeProcess() hooks
 * ===========================================
 *
 * These advanced hooks might be used for additional MPI communication between nodes, or
 * to allocate additional data structures that cannot be passed directly through the API. The
 * all configuration and data for the current, and previous task pools is passed.
 *
 * The NodePrepare() hook is invoked before PoolPrepare().
 * The NodeProcess() hook is invoked after PoolProcess().
 * 
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_node.c -o libmechanic_module_ex_node.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_node -x 10 -y 20
 *
 */
#include "mechanic.h"

/* Sample global struct */
typedef struct pass {
  double data;
  int node;
} pass;

/* The master node will allocate mpi_size, workers only 1 */  
pass *g;

/**
 * Implements NodePrepare()
 */
int NodePrepare(int mpi_size, int node, pool **all, pool *p, void *s) {
  double data;

  if (node == MASTER) {
    data = 99.97;
  }

  // Broadcast some simple data
  MPI_Bcast(&data, 1, MPI_DOUBLE, MASTER, MPI_COMM_WORLD);

  Message(MESSAGE_OUTPUT, "Node %d data = %f\n", node, data);

  /* Allocate the global struct */
  if (node == MASTER) {
    g = malloc(mpi_size * sizeof(pass));
    if (!g) return MODULE_ERR_MEM;
  } else {
    g = malloc(sizeof(pass));
    if (!g) return MODULE_ERR_MEM;
  }

  if (node == MASTER) {
    g[0].data = data;
    g[0].node = node;
  } else {
    g->data = data;
    g->node = node;
  } 
  return SUCCESS;
} 

/**
 * PoolPrepare()
 *    LoopPrepare()
 *      TaskPrepare()
 *      TaskProcess()
 *    LoopProcess()
 * PoolProcess()
 */

/**
 * Implements NodeProcess()
 *
 * Here, we free the previously allocated struct. We do this only when the pool is about
 * to finish (not reset).
 */
int NodeProcess(int mpi_size, int node, pool **all, pool *p, void *s) {

  if (node != MASTER) {
    Message(MESSAGE_OUTPUT, "g->node %d, g->data = %f\n", g->node, g->data);
  }

  if (p->status == POOL_CREATE_NEW || p->status == POOL_FINALIZE) {
    free(g);
  }

  return SUCCESS;
} 

