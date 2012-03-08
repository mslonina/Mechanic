/**
 * @file
 * The core-related functions.
 */
#include "MCore.h"

/**
 * @function
 * Wrapper to MPI_Abort
 */
int Abort(int errcode) {
  MPI_Abort(MPI_COMM_WORLD, errcode);

  return 0;
}

