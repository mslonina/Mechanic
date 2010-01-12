#include "mpifarm.h"
#include "mpifarm-internals.h"

/**
 * HDF5 FUNCTIONS
 */

/* Error handling */
void H5errcheck(hid_t handler, herr_t stat, char* message, int rank){
  return;
}

/* H5 logs */
void H5log(){
  return;
}

/* Master data scheme */
void H5createMasterDataScheme(hid_t file_id, configData* d){

 hsize_t dimsf[2], dimsr[2];
 hid_t boardspace, dataspace;
 hid_t dset_board, data_group, dset_data;

 /* Control board space */
 dimsf[0] = d->xres;
 dimsf[1] = d->yres;
 boardspace = H5Screate_simple(HDF_RANK, dimsf, NULL);
 
 dset_board = H5Dcreate(file_id, DATABOARD, H5T_NATIVE_INT, boardspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

 /* Master data group */
 data_group = H5Gcreate(file_id, DATAGROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
 
 /* Result data space  */
 dimsr[0] = d->xres*d->yres;
 dimsr[1] = d->mrl;
 dataspace = H5Screate_simple(HDF_RANK, dimsr, NULL);

 /* Create master dataset */
 dset_data = H5Dcreate(data_group, DATASETMASTER, H5T_NATIVE_DOUBLE, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

 H5Dclose(dset_board);
 H5Dclose(dset_data);
 H5Sclose(boardspace);
 H5Sclose(dataspace);
 H5Gclose(data_group);

}

/* Write data to master file */
void H5writeMaster(hid_t dset, hid_t memspace, hid_t space, configData* d, int* coordsarr, MY_DATATYPE* resultarr){
  
  MY_DATATYPE rdata[d->mrl][1];
  hsize_t co[2], off[2];
  herr_t hdf_status;
  int j = 0;
      
  co[0] = 1;
  co[1] = d->mrl;

  off[0] = coordsarr[2];
  off[1] = 0;
  
  for (j = 0; j < d->mrl; j++){
    rdata[j][0] = resultarr[j];
  }
      
  H5Sselect_hyperslab(space, H5S_SELECT_SET, off, NULL, co, NULL);
  hdf_status = H5Dwrite(dset, H5T_NATIVE_DOUBLE, memspace, space, H5P_DEFAULT, rdata);

}

/* Mark computed pixels on board */
void H5writeBoard(hid_t dset, hid_t memspace, hid_t space, int* coordsarr){
  
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

}

/* Write checkpoint file (master file) */
void H5writeCheckPoint(configData *d, int check, int** coordsarr, MY_DATATYPE** resultarr){
 
  int i = 0, j = 0; 

  /* hdf */
  hid_t file_id, dset_board, dset_data, data_group;
  hid_t mapspace, memmapspace, rawspace, memrawspace, maprawspace; 
  hsize_t co[2], rco[2], off[2];
  herr_t hdf_status;

  /* Open file */
  file_id = H5Fopen(d->datafile,H5F_ACC_RDWR,H5P_DEFAULT);
  dset_board = H5Dopen(file_id, DATABOARD, H5P_DEFAULT);   
  data_group = H5Gopen(file_id, DATAGROUP, H5P_DEFAULT);
  dset_data = H5Dopen(data_group, DATASETMASTER, H5P_DEFAULT);   
 
  /* We write pixels one by one */
  co[0] = 1;
  co[1] = 1;
  memmapspace = H5Screate_simple(HDF_RANK, co, NULL);

  rco[0] = 1;
  rco[1] = d->mrl;
  memrawspace = H5Screate_simple(HDF_RANK, rco, NULL);
 
  mapspace = H5Dget_space(dset_board);
  rawspace = H5Dget_space(dset_data);

  /* Write data */
  for(i = 0; i < check; i++){
      
      /* Control board -- each computed pixel is marked with 1. */
      H5writeBoard(dset_board, memmapspace, mapspace, coordsarr[i]);
      
      /* Data */
      H5writeMaster(dset_data, memrawspace, rawspace, d, coordsarr[i], resultarr[i]);
      
  }

  H5Dclose(dset_board);
  H5Dclose(dset_data);
  H5Sclose(mapspace);
  H5Sclose(memmapspace);
  H5Sclose(rawspace);
  H5Sclose(memrawspace);
  H5Gclose(data_group);
  H5Fclose(file_id);

}
