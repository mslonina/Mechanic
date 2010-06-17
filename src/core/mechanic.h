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

#ifndef MECHANIC_H
#define MECHANIC_H

#include "libreadconfig.h"

#define MECHANIC_ERR_MPI 911
#define MECHANIC_ERR_HDF 912
#define MECHANIC_ERR_MODULE 913
#define MECHANIC_ERR_SETUP 914
#define MECHANIC_ERR_MEM 915
#define MECHANIC_ERR_CHECKPOINT 916
#define MECHANIC_ERR_OTHER 999

#define MECHANIC_HDF_RANK 2

#define MECHANIC_STRLEN 256

#undef MECHANIC_DATATYPE
#define MECHANIC_DATATYPE double

#undef MECHANIC_MPI_DATATYPE
#define MECHANIC_MPI_DATATYPE MPI_DOUBLE

#define ITEMS_IN_ARRAY(x) sizeof(x)/sizeof(*(x))

enum Messages{
  MECHANIC_MESSAGE_INFO,
  MECHANIC_MESSAGE_ERR,
  MECHANIC_MESSAGE_IERR,
  MECHANIC_MESSAGE_CONT,
  MECHANIC_MESSAGE_WARN,
	MECHANIC_MESSAGE_DEBUG
} mechanicMessages;

/* [CONFIGDATA] */
typedef struct {
  char name[MECHANIC_STRLEN];
  char datafile[MECHANIC_STRLEN];
  char module[MECHANIC_STRLEN];
  int xres;
  int yres;
  int method;
  int checkpoint;
  int restartmode;
  int mode;
} configData;
/* [/CONFIGDATA] */

/* [MASTERDATA] */
typedef struct {
  MECHANIC_DATATYPE *res;
  int coords[3]; /* 0 - x 1 - y 2 - number of the pixel */
} masterData;
/* [/MASTERDATA] */

/* [MODULEINFO] */
typedef struct {
  int irl;
  int mrl;
} moduleInfo;
/* [/MODULEINFO] */

void mechanic_message(int type, char* fmt, ...);
int mechanic_finalize(int node);
int mechanic_abort(int errcode);
void mechanic_error(int stat);

#endif

