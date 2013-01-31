/**
 * @file
 * Public task pool interface
 */
#ifndef MECHANIC_PUBLIC_POOL_H
#define MECHANIC_PUBLIC_POOL_H

/* Pool */
#define POOL_FINALIZE 1001 /**< The pool finalize return code */
#define POOL_RESET 1002 /**< The pool reset return code */
#define POOL_CREATE_NEW 1003 /**< The pool create new return code */
#define POOL_PREPARED 2001 /**< Pool prepared state */
#define POOL_PROCESSED 2002 /**< Pool processed state */

/**
 * @struct pool
 * The pool
 */
typedef struct {
  int pid; /**< The pool id */
  int rid; /**< The reset id */
  int status; /**< The pool create status */
  int state; /**< The pool processing state */
  storage *board; /**< The task board */
  storage *storage; /**< The global pool storage scheme */
  task *task; /**< The task scheme */
  task **tasks; /**< All tasks */
  int checkpoint_size; /**< The checkpoint size */
  int pool_size; /**< The pool size (number of tasks to do) */
  int completed; /**< The pool task completed counter */
  int node; /**< The node ID */
  int mpi_size; /**< The MPI COMM size */
} pool;

int ReadPool(pool *p, char *storage_name, void *data); /**< Read pool data */
int WritePool(pool *p, char *storage_name, void *data); /**< Write data to the pool */

#endif

