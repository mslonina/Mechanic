/**
 * Using RNGS library
 * ==================
 *
 * This example shows simple implementation of the RNGS library that ships with the
 * Mechanic. The library provides the global `seed` array, which is used during random
 * number generation. It is possible to use different discrete distributions, see:
 *
 * http://www.cs.wm.edu/~va/software/park/park.html
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_rngs.c -o libmechanic_module_ex_rngs.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_rngs -x 10 -y 20
 *
 */
#include "mechanic.h"

/**
 * Implements Prepare()
 */
int Prepare(int node, char *masterfile, void *s) {
  long x = -1;
  
  // Initialize random seeds
  PlantSeeds(x);

  // Generate random numbers
  Random();

  return SUCCESS;
}
