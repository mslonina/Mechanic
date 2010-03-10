/*
 * MECHANIC Copyright (c) 2010, Mariusz Slonina (Nicolaus Copernicus University)
 * All rights reserved.
 * 
 * This file is part of MECHANIC code. 
 *
 * MECHANIC was created to help solving many numerical problems by providing tools
 * for improving scalability and functionality of the code. MECHANIC was released 
 * in belief it will be useful. If you are going to use this code, or its parts,
 * please consider referring to the authors either by the website or the user guide 
 * reference.
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
 *  - Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *  - Neither the name of the Nicolaus Copernicus University nor the names of 
 *    its contributors may be used to endorse or promote products derived from 
 *    this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#include "mechanic.h"
#include "mechanic_internals.h"

/*
 * MPI DERIVED DATATYPES
 */

// Master result Send/Recv 
int buildMasterResultsType(int mrl, masterData* md, MPI_Datatype* masterResultsType_ptr){

  int block_lengths[2];
  MPI_Aint displacements[2];
  MPI_Datatype typelist[2];
  MPI_Aint addresses[3];

  typelist[0] = MPI_DOUBLE;
  typelist[1] = MPI_INT;

  block_lengths[0] = mrl;
  block_lengths[1] = 3;

  MPI_Address(md, &addresses[0]);
  MPI_Address(md->res, &addresses[1]);
  MPI_Address(md->coords, &addresses[2]);

  displacements[0] = addresses[1] - addresses[0];
  displacements[1] = addresses[2] - addresses[0];

  MPI_Type_struct(2, block_lengths, displacements, typelist, masterResultsType_ptr);
  MPI_Type_commit(masterResultsType_ptr);

  return 0;
}

// Bcast default config file 
int buildDefaultConfigType(configData* d, MPI_Datatype* defaultConfigType_ptr){
  
  int block_lengths[10];
  MPI_Aint displacements[10];
  MPI_Datatype typelist[10];
  MPI_Aint addresses[11];
  int i = 0;

  typelist[0] = MPI_CHAR; //problem name
  typelist[1] = MPI_CHAR; //master datafile
  typelist[2] = MPI_CHAR; //module
  typelist[3] = MPI_INT; //xres
  typelist[4] = MPI_INT; //yres
  typelist[5] = MPI_INT; //method
  typelist[6] = MPI_INT; //checkpoint
  typelist[7] = MPI_INT; //restartmode
  typelist[8] = MPI_INT; //mode
  typelist[9] = MPI_INT; //checkpoint number

  block_lengths[0] = 256;
  block_lengths[1] = 260;
  block_lengths[2] = 256;
  block_lengths[3] = 1;
  block_lengths[4] = 1;
  block_lengths[5] = 1;
  block_lengths[6] = 1;
  block_lengths[7] = 1;
  block_lengths[8] = 1;
  block_lengths[9] = 1;

  MPI_Address(d, &addresses[0]);
  MPI_Address(&(d->name), &addresses[1]);
  MPI_Address(&(d->datafile), &addresses[2]);
  MPI_Address(&(d->module), &addresses[3]);
  MPI_Address(&(d->xres), &addresses[4]);
  MPI_Address(&(d->yres), &addresses[5]);
  MPI_Address(&(d->method), &addresses[6]);
  MPI_Address(&(d->checkpoint), &addresses[7]);
  MPI_Address(&(d->restartmode), &addresses[8]);
  MPI_Address(&(d->mode), &addresses[9]);
  MPI_Address(&(d->checkpoint_num), &addresses[10]);

  for(i = 0; i < 10; i++){
    displacements[i] = addresses[i+1] - addresses[0];
  }

  MPI_Type_struct(10, block_lengths, displacements, typelist, defaultConfigType_ptr);
  MPI_Type_commit(defaultConfigType_ptr);

  return 0;
}
