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
 * HDF5 FUNCTIONS
 */

// Error handling 
void H5errcheck(hid_t handler, herr_t stat, char* message, int rank){
  return;
}

// H5 logs 
void H5log(){
  return;
}

// Master data scheme 
int H5createMasterDataScheme(hid_t file_id, moduleInfo *md, configData* d){

 hsize_t dimsf[2], dimsr[2];
 hid_t boardspace, dataspace;
 hid_t dset_board, data_group, dset_data;

 // Control board space 
 dimsf[0] = d->xres;
 dimsf[1] = d->yres;
 boardspace = H5Screate_simple(MECHANIC_HDF_RANK, dimsf, NULL);
 
 dset_board = H5Dcreate(file_id, MECHANIC_DATABOARD, H5T_NATIVE_INT, boardspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

 // Master data group
 data_group = H5Gcreate(file_id, MECHANIC_DATAGROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
 
 // Result data space 
 dimsr[0] = d->xres*d->yres;
 dimsr[1] = md->mrl;
 dataspace = H5Screate_simple(MECHANIC_HDF_RANK, dimsr, NULL);

 // Create master dataset 
 dset_data = H5Dcreate(data_group, MECHANIC_DATASETMASTER, H5T_NATIVE_DOUBLE, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

 H5Dclose(dset_board);
 H5Dclose(dset_data);
 H5Sclose(boardspace);
 H5Sclose(dataspace);
 H5Gclose(data_group);

 return 0;
}

// Write data to master file 
int H5writeMaster(hid_t dset, hid_t memspace, hid_t space, moduleInfo *md, configData* d, int* coordsarr, MECHANIC_DATATYPE* resultarr){
  
  MECHANIC_DATATYPE rdata[md->mrl][1];
  hsize_t co[2], off[2];
  herr_t hdf_status;
  int j = 0;
      
  co[0] = 1;
  co[1] = md->mrl;

  off[0] = coordsarr[2];
  off[1] = 0;
  
  for (j = 0; j < md->mrl; j++){
    rdata[j][0] = resultarr[j];
  }
      
  H5Sselect_hyperslab(space, H5S_SELECT_SET, off, NULL, co, NULL);
  hdf_status = H5Dwrite(dset, H5T_NATIVE_DOUBLE, memspace, space, H5P_DEFAULT, rdata);

  return 0;
}

// Mark computed pixels on board 
int H5writeBoard(hid_t dset, hid_t memspace, hid_t space, int* coordsarr){
  
  int rdata[1][1];
  hsize_t co[2], off[2];
  herr_t hdf_status;
      
  co[0] = 1;
  co[1] = 1;

  off[0] = coordsarr[0];
  off[1] = coordsarr[1];
 
  rdata[0][0] = 1;
      
  H5Sselect_hyperslab(space, H5S_SELECT_SET, off, NULL, co, NULL);
  hdf_status = H5Dwrite(dset, H5T_NATIVE_INT, memspace, space, H5P_DEFAULT, rdata);

  return 0;
}

