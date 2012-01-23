/**
 * @file
 * The Error reporting subsystem
 */
#include "mechanic.h"
#include "mechanic_internals.h"

/* [ERRORS] */

/**
 * @section bugs Known Bugs and Missing Features
 *
 * Known bugs and missing features:
 *
 * - HDF error handling in a better way
 *
 * @section errcodes Error Codes
 *
 * In case of emergency @M tries to properly finalize all nodes and returns
 * error codes as described below:
 *
 * - @c 911 -- MPI related error
 * - @c 912 -- HDF related error
 * - @c 913 -- TaskInfo subsystem related error
 * - @c 914 -- Setup subsystem related error
 * - @c 915 -- Memory allocation related error
 * - @c 916 -- Checkpoint subsytem related error
 * - @c 999 -- Any other error
 *
 * You can pass module errors in a similar way, by using proper errcodes as a returned
 * value from a function:
 *
 * - @c 811 -- MPI related error
 * - @c 812 -- HDF related error
 * - @c 813 -- TaskInfo subsystem related error
 * - @c 814 -- Setup subsystem related error
 * - @c 815 -- Memory allocation related error
 * - @c 816 -- Checkpoint subsytem related error
 * - @c 888 -- Any other error
 *
 */

/* [/ERRORS] */

/* HDF error handler */
int mechanic_error_hdf(herr_t errcode) {
  if (errcode < 0) {
    H5Eprint(H5E_DEFAULT, stderr);
    return MECHANIC_ERR_HDF;
  }
  return 0;
}

/* Mechanic error handler */
void mechanic_error(int errcode){

  /* We abort on any memory-type error, i.e. wrong mallocs */
  if (errcode == MECHANIC_ERR_MEM) mechanic_abort(MECHANIC_ERR_MEM);
  if (errcode == MECHANIC_MODULE_ERR_MEM) mechanic_abort(MECHANIC_MODULE_ERR_MEM);

  /* Abort on any setup error,
   * i.e. config file was not found and option -c is set */
  if (errcode == MECHANIC_ERR_SETUP) {
		mechanic_message(MECHANIC_MESSAGE_ERR,"Error opening config file:");
    mechanic_abort(MECHANIC_ERR_SETUP);
  }
  if (errcode == MECHANIC_MODULE_ERR_SETUP) {
		mechanic_message(MECHANIC_MESSAGE_ERR,"TaskInfo setup error");
    mechanic_abort(MECHANIC_MODULE_ERR_SETUP);
  }

  /* We abort on any hdf-related error, i.e. error during data writing */
  if (errcode == MECHANIC_ERR_HDF) mechanic_abort(MECHANIC_ERR_HDF);
  if (errcode == MECHANIC_MODULE_ERR_HDF) mechanic_abort(MECHANIC_MODULE_ERR_HDF);

  /* We abort on any module-related error, i.e. module was not found */
  if (errcode == MECHANIC_ERR_MODULE) mechanic_abort(MECHANIC_ERR_MODULE);
  if (errcode == MECHANIC_MODULE_ERR_MODULE) mechanic_abort(MECHANIC_MODULE_ERR_MODULE);

  /* We abort on any other less than zero err */
  if (errcode < MECHANIC_TASK_SUCCESS) mechanic_abort(MECHANIC_ERR_OTHER);

  /* We abort on MECHANIC_ICE signal */
  if (errcode == MECHANIC_ICE) mechanic_abort(MECHANIC_ICE);

}

/* Wrapper for error messages */
void mechanic_check_mstat(int errcode) {
  mechanic_error(errcode);
}
