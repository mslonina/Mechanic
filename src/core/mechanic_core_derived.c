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

/* MPI DERIVED DATATYPES */

/* Master result Send/Recv */
int buildMasterResultsType(int mrl, masterData* md,
    MPI_Datatype* masterResultsType_ptr){

  int mstat;
  int block_lengths[2];
  MPI_Aint displacements[2];
  MPI_Datatype typelist[2];
  MPI_Aint addresses[3];

  typelist[0] = MPI_DOUBLE;
  typelist[1] = MPI_INT;

  block_lengths[0] = mrl; 
  block_lengths[1] = 3;

  MPI_Get_address(md, &addresses[0]);
  MPI_Get_address(md->res, &addresses[1]);
  MPI_Get_address(md->coords, &addresses[2]);

  displacements[0] = addresses[1] - addresses[0];
  displacements[1] = addresses[2] - addresses[0];

  mstat = MPI_Type_struct(2, block_lengths, displacements, typelist,
    masterResultsType_ptr);
  mstat = MPI_Type_commit(masterResultsType_ptr);

  return mstat;
}

/* Configuration datatype */
int buildConfigDataType(int lengths[4], configData d, MPI_Datatype* configDataType_ptr) {

  int mstat;
  int i;
  int block_lengths[11];
  MPI_Aint displacements[11];
  MPI_Datatype typelist[11];
  MPI_Aint addresses[12];

  mstat = 0;

  block_lengths[0] = MECHANIC_STRLEN; typelist[0] = MPI_CHAR;
  block_lengths[1] = MECHANIC_STRLEN; typelist[1] = MPI_CHAR;
  block_lengths[2] = MECHANIC_STRLEN; typelist[2] = MPI_CHAR;
  block_lengths[3] = 1; typelist[3] = MPI_INT;
  block_lengths[4] = 1; typelist[4] = MPI_INT;
  block_lengths[5] = 1; typelist[5] = MPI_INT;
  block_lengths[6] = 1; typelist[6] = MPI_INT;
  block_lengths[7] = 1; typelist[7] = MPI_INT;
  block_lengths[8] = 1; typelist[8] = MPI_INT;
  block_lengths[9] = 1; typelist[9] = MPI_INT;
  block_lengths[10] = 1; typelist[10] = MPI_INT;

  MPI_Get_address(&d, &addresses[0]);
  MPI_Get_address(&d.name, &addresses[1]);
  MPI_Get_address(&d.datafile, &addresses[2]);
  MPI_Get_address(&d.module, &addresses[3]);
  MPI_Get_address(&d.name_len, &addresses[4]);
  MPI_Get_address(&d.datafile_len, &addresses[5]);
  MPI_Get_address(&d.module_len, &addresses[6]);
  MPI_Get_address(&d.xres, &addresses[7]);
  MPI_Get_address(&d.yres, &addresses[8]);
  MPI_Get_address(&d.checkpoint, &addresses[9]);
  MPI_Get_address(&d.restartmode, &addresses[10]);
  MPI_Get_address(&d.mode, &addresses[11]);

  for (i = 0; i < 11; i++) {
    displacements[i] = addresses[i+1] - addresses[0];
  }

  mstat = MPI_Type_struct(11, block_lengths, displacements, typelist,
    configDataType_ptr);
  mstat = MPI_Type_commit(configDataType_ptr);

  return mstat;
}
