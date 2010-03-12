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

int atCheckPoint(int check, int** coordsarr, int** board, MECHANIC_DATATYPE** resultarr, moduleInfo* md, configData* d){
  
  int mstat;

  mstat = manageCheckPoints(d);

  mstat = H5writeCheckPoint(md, d, check, coordsarr, resultarr);
  
  return 0;
}

int manageCheckPoints(configData* d){

  int i;
  char checkpoint[MECHANIC_FILE+6];
  char checkpoint_old[MECHANIC_FILE+6];

  sprintf(checkpoint,"%s-cp%03d.h5", d->name, MECHANIC_CHECKPOINTS-1);

  for(i = MECHANIC_CHECKPOINTS-2; i >= 0; i--){
    sprintf(checkpoint,"%s-cp%03d.h5", d->name, i+1);
    sprintf(checkpoint_old,"%s-cp%03d.h5", d->name, i);
    /* rename(checkpoint_old, checkpoint); */
    /*  printf("File %s -> %s\n",checkpoint_old, checkpoint); */
  }

  /* printf("File %s will be copied to %s and used\n", checkpoint, checkpoint_old); */
  return 0;
}

/* Write checkpoint file (master file) */
int H5writeCheckPoint(moduleInfo *md, configData *d, int check, int** coordsarr, MECHANIC_DATATYPE** resultarr){
 
  int i = 0;
  int mstat;

  /* HDF */
  hid_t file_id, dset_board, dset_data, data_group;
  hid_t mapspace, memmapspace, rawspace, memrawspace; 
  hsize_t co[2], rco[2];

  /* Open file */
  file_id = H5Fopen(d->datafile,H5F_ACC_RDWR,H5P_DEFAULT);
	
	mechanic_message(MECHANIC_MESSAGE_DEBUG, "Checkpoint file: %s\n", d->datafile); 
  
	dset_board = H5Dopen(file_id, MECHANIC_DATABOARD, H5P_DEFAULT);   
  data_group = H5Gopen(file_id, MECHANIC_DATAGROUP, H5P_DEFAULT);
  dset_data = H5Dopen(data_group, MECHANIC_DATASETMASTER, H5P_DEFAULT);   
 
  /* We write pixels one by one */
  co[0] = 1;
  co[1] = 1;
  memmapspace = H5Screate_simple(MECHANIC_HDF_RANK, co, NULL);

  rco[0] = 1;
  rco[1] = md->mrl;
  memrawspace = H5Screate_simple(MECHANIC_HDF_RANK, rco, NULL);
 
  mapspace = H5Dget_space(dset_board);
  rawspace = H5Dget_space(dset_data);

  /* Write data */
  for(i = 0; i < check; i++){
      
      /* Control board -- each computed pixel is marked with 1. */
      mstat = H5writeBoard(dset_board, memmapspace, mapspace, coordsarr[i]);
      
      /* Data */
      mstat = H5writeMaster(dset_data, memrawspace, rawspace, md, d, coordsarr[i], resultarr[i]);
      
  }

  H5Dclose(dset_board);
  H5Dclose(dset_data);
  H5Sclose(mapspace);
  H5Sclose(memmapspace);
  H5Sclose(rawspace);
  H5Sclose(memrawspace);
  H5Gclose(data_group);
  H5Fclose(file_id);
	
	mechanic_message(MECHANIC_MESSAGE_DEBUG,"Checkpoint finished\n"); 

  return 0;
}

/*
 * READ BOARD AT RESTARTMODE
 *
 * FIX ME!
 * I couldn't manage to do it with single call to H5Dread and malloc,
 * so I select pixels one by one and read them to board array
 */
int H5readBoard(configData* d, int** board){

  hid_t file_id, dataset_id;
  hid_t dataspace_id, memspace_id;
  hsize_t co[2], offset[2], stride[2], block[2], dims[2];
  herr_t hdf_status;

  int rdata[1][1];
  int i = 0, j = 0;
  
  /* Open checkpoint file */
  file_id = H5Fopen(d->datafile, H5F_ACC_RDONLY, H5P_DEFAULT);
  dataset_id = H5Dopen(file_id, MECHANIC_DATABOARD, H5P_DEFAULT);
  dataspace_id = H5Dget_space(dataset_id);
  
  /* Create memory space for one by one pixel read */
  dims[0] = 1;
  dims[1] = 1;
  memspace_id = H5Screate_simple(MECHANIC_HDF_RANK, dims, NULL);

  /* Read board pixels one by one */
  for(i = 0; i < d->xres; i++){
    for(j = 0; j < d->yres; j++){

      offset[0] = i;
      offset[1] = j;
      
      co[0] = 1;
      co[1] = 1;
      
      stride[0] = 1;
      stride[1] = 1;

      block[0] = 1;
      block[1] = 1;

      rdata[0][0] = 0;

      hdf_status = H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, offset, stride, co, block);

      hdf_status = H5Dread(dataset_id, H5T_NATIVE_INT, memspace_id, dataspace_id, H5P_DEFAULT, rdata);

      /* Copy temporary data array to board array */
      board[i][j] = rdata[0][0];
      
    }
  }

  /* Close checkpoint file */
  H5Sclose(memspace_id);
  H5Sclose(dataspace_id);
  H5Dclose(dataset_id);
  H5Fclose(file_id);
 
  return 0;
}

