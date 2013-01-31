/**
 * Overriding the pool size
 * ========================
 *
 * This example shows very simple way to override the pool size. Despite of the run
 * resolution, we set the pool size to 4096x1.
 * 
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_poolsize.c -o libmechanic_module_ex_poolsize.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_poolsize -x 10 -y 20
 *
 */

#include "mechanic.h"

/**
 * Implements Storage()
 */
int Storage(pool *p, void *s) {
  // Override the pool size
  p->pool_size = 4096;

  // Important! We must set the task board to proper size
  p->board->layout.dims[0] = p->pool_size;
  p->board->layout.dims[1] = 1;

  return SUCCESS;
}

