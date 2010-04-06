/*
 * MECHANIC
 * Copyright (c) 2010, Mariusz Slonina (Nicolaus Copernicus University)
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
 * http://mechanics.astri.umk.pl/projects/mechanic
 *
 * User guide should be provided with the package or
 * http://mechanics.astri.umk.pl/projects/mechanic/mechanic_userguide.pdf
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
 * @section bugs Known bugs and missing features
 *
 * Known bugs and missing features:
 *
 * - restart/checkpoint system
 * - HDF error handling in a better way
 * - extenstion to the template system: overrides specific for given node
 *
 * @section errcodes Error codes
 *
 * In case of emergency @M tries to properly finalize all nodes and returns
 * error codes as described below:
 *
 * - @c 911 -- MPI related error
 * - @c 912 -- HDF related error
 * - @c 913 -- Module interface related error
 * - @c 914 -- Setup interface related error
 * - @c 915 -- Memory allocation related error
 * - @c 999 -- Any other error
 *
 */

/* [/ERRORS] */

/* H5 logs */
void H5log(){
  return;
}

/* Mechanic error handler */
void mechanic_error(int errcode){

  /* We abort on any memory-type error, i.e. wrong mallocs */
  if (errcode == MECHANIC_ERR_MEM) mechanic_abort(MECHANIC_ERR_MEM);

  /* Abort on any setup error,
   * i.e. config file was not found and option -c is set */
  if (errcode == MECHANIC_ERR_SETUP) {
		mechanic_message(MECHANIC_MESSAGE_ERR,"Error opening config file:");
    mechanic_abort(MECHANIC_ERR_SETUP);
  }

  /* We abort on any hdf-related error, i.e. error during data writing */
  if (errcode == MECHANIC_ERR_HDF) mechanic_abort(MECHANIC_ERR_HDF);

  /* We abort on any module-related error, i.e. module was not found */
  if (errcode == MECHANIC_ERR_MODULE) mechanic_abort(MECHANIC_ERR_MODULE);

  return;
}
