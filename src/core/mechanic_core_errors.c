/*
 * MECHANIC
 *
 * Copyright (c) 2010-2011, Mariusz Slonina (Nicolaus Copernicus University)
 * All rights reserved.
 *
 * This file is part of MECHANIC code.
 *
 * MECHANIC was created to help solving many numerical problems by providing
 * tools for improving scalability and functionality of the code. MECHANIC was
 * released in belief it will be useful. If you are going to use this code, or
 * its parts, please consider referring to the authors either by the website
 * or the user guide reference.
 *
 * http://git.astri.umk.pl/projects/mechanic
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the Nicolaus Copernicus University nor the names of
 *   its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
 * - @c 913 -- Module subsystem related error
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
 * - @c 813 -- Module subsystem related error
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
		mechanic_message(MECHANIC_MESSAGE_ERR,"Module setup error");
    mechanic_abort(MECHANIC_MODULE_ERR_SETUP);
  }

  /* We abort on any hdf-related error, i.e. error during data writing */
  if (errcode == MECHANIC_ERR_HDF) mechanic_abort(MECHANIC_ERR_HDF);
  if (errcode == MECHANIC_MODULE_ERR_HDF) mechanic_abort(MECHANIC_MODULE_ERR_HDF);

  /* We abort on any module-related error, i.e. module was not found */
  if (errcode == MECHANIC_ERR_MODULE) mechanic_abort(MECHANIC_ERR_MODULE);
  if (errcode == MECHANIC_MODULE_ERR_MODULE) mechanic_abort(MECHANIC_MODULE_ERR_MODULE);

  /* We abort on any other less than zero err */
  if (errcode < 0) mechanic_abort(MECHANIC_ERR_OTHER);

  /* We abort on MECHANIC_ICE signal */
  if (errcode == MECHANIC_ICE) mechanic_abort(MECHANIC_ICE);

}

/* Wrapper for error messages */
void mechanic_check_mstat(int errcode) {
  mechanic_error(errcode);
}
