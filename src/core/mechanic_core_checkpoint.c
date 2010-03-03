#include "mechanic.h"
#include "mechanic_internals.h"

int atCheckPoint(int check, int** coordsarr, int** board, MECHANIC_DATATYPE** resultarr, moduleInfo *md, configData* d){
  
  int i, mstat;

  mstat = manageCheckPoints(d);

  mstat = H5writeCheckPoint(md, d, check, coordsarr, resultarr);
  
  //printf("At checkpoint\n");
  return 0;
}

int manageCheckPoints(configData* d){

  int mstat;
  int i;
  char checkpoint[MECHANIC_FILE+6];
  char checkpoint_old[MECHANIC_FILE+6];

  sprintf(checkpoint,"%s-cp%03d.h5", d->name, MECHANIC_CHECKPOINTS-1);
  //printf("File %s will be removed\n", checkpoint);

  for(i = MECHANIC_CHECKPOINTS-2; i >= 0; i--){
    sprintf(checkpoint,"%s-cp%03d.h5", d->name, i+1);
    sprintf(checkpoint_old,"%s-cp%03d.h5", d->name, i);
    //rename(checkpoint_old, checkpoint);
  //  printf("File %s -> %s\n",checkpoint_old, checkpoint);
  }

  //printf("File %s will be copied to %s and used\n", checkpoint, checkpoint_old);
  return 0;
}

/* Write checkpoint file (master file) */
int H5writeCheckPoint(moduleInfo *md, configData *d, int check, int** coordsarr, MECHANIC_DATATYPE** resultarr){
 
  int i = 0, j = 0;
  int mstat;

  /* hdf */
  hid_t file_id, dset_board, dset_data, data_group;
  hid_t mapspace, memmapspace, rawspace, memrawspace, maprawspace; 
  hsize_t co[2], rco[2], off[2];
  herr_t hdf_status;

  /* Open file */
  file_id = H5Fopen(d->datafile,H5F_ACC_RDWR,H5P_DEFAULT);
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

  return 0;
}

/**
 * READ BOARD AT RESTARTMODE
 *
 * FIX ME!
 * I couldn't manage to do it with single call to H5Dread and malloc,
 * so I select pixels one by one and read them to board array
 * */
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
