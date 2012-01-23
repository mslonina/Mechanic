/**
 * @file
 * The Checkpoint Subsystem
 */
#include "mechanic.h"
#include "mechanic_internals.h"

/* [CHECKPOINT] */

/**
 * @section checkpoint Checkpoints
 * @M comes with integrated checkpoint system, which helps with master file
 * backup and restarting simulations. By default, the checkpoint file write
 * interval is setuped to 2000, which means, that data will be stored in the
 * master file after each 2000 pixel have been reached. You can change this
 * interval by setting @c checkpoint in config file or using @c --checkpoint
 * @c -d in the command line.
 *
 * @M will create up to 3 checkpoint file, in the well-known incremental backup
 * system. Each file will have a corresponding checkpoint number (starting from
 * the master file, 00, up to 02).
 *
 * You can use any of the checkpoint files to restart your simulation. To use
 * restart mode, try @c --restart or @c -r command line option and provide the
 * path to the checkpoint file to use. If the file is not usable, @M will
 * abort.
 *
 * In restart mode @M will do only not previously finished simulations. At this
 * stage of development, it is not possible to restart partially done
 * simulations.
 *
 */

/* [/CHECKPOINT] */

int atCheckPoint(mechanic_internals *handler, int check, int** coordsarr, int** board,
    MECHANIC_DATATYPE** resultarr) {

  int mstat = 0;

  /* Create incremental set of checkpoint file */
  mstat = manageCheckPoints(handler->config);
  mechanic_check_mstat(mstat);

  /* Validate current checkpoint file */
  mstat = mechanic_validate_file(handler->config->datafile);
  mechanic_check_mstat(mstat);

  /* Write data to checkpoint file */
  mstat = H5writeCheckPoint(handler->info, handler->config, check, coordsarr, resultarr);
  mechanic_check_mstat(mstat);

  return mstat;
}

int manageCheckPoints(TaskConfig* d){

  int i, mstat = 0;
  char *checkpoint;
  char *checkpoint_old;
  size_t nam, pre, ext, clen;

  struct stat ct;
  struct stat cto;

  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Managing checkpoints\n");

  /* Allocate memory for names of files */
  nam = strlen(d->name);
  pre = strlen(MECHANIC_MASTER_PREFIX_DEFAULT);
  ext = strlen(MECHANIC_FILE_EXT);

  clen = nam + pre + ext + 6*sizeof(char*);

  checkpoint = calloc(clen, sizeof(char*));
  if (checkpoint == NULL) return MECHANIC_ERR_MEM;

  checkpoint_old = calloc(clen, sizeof(char*));
  if (checkpoint_old == NULL) return MECHANIC_ERR_MEM;

  for (i = MECHANIC_CHECKPOINTS-2; i >= 0; i--) {

    snprintf(checkpoint, clen, "%s-%s-%02d.%s",
        d->name, MECHANIC_MASTER_PREFIX_DEFAULT, i+1, MECHANIC_FILE_EXT);
    snprintf(checkpoint_old, clen, "%s-%s-%02d.%s",
        d->name, MECHANIC_MASTER_PREFIX_DEFAULT, i, MECHANIC_FILE_EXT);

    if (stat(checkpoint_old, &cto) == 0 ) {
      if (stat(checkpoint, &ct) < 0) {

        /* If the checkpoint file doesn't exist, copy current file */
        mstat = mechanic_copy(checkpoint_old, checkpoint);
        if (mstat < 0) return MECHANIC_ERR_CHECKPOINT;

      } else {

        /* Instead of renaming, if we reach 00 file, we copy it to 01 */
        if (i == 0) {
          mstat = mechanic_copy(checkpoint_old, checkpoint);
          if (mstat != 0) return MECHANIC_ERR_CHECKPOINT;
        } else {
          mstat = rename(checkpoint_old, checkpoint);
          if (mstat < 0) return MECHANIC_ERR_CHECKPOINT;
          mechanic_message(MECHANIC_MESSAGE_DEBUG,
              "Renamed file size = %d\n", mstat);
        }

      }

      mechanic_message(MECHANIC_MESSAGE_DEBUG,
          "File %s -> %s\n",checkpoint_old, checkpoint);
    }
  }

  free(checkpoint);
  free(checkpoint_old);

  return mstat;
}

/* Write checkpoint file (master file) */
int H5writeCheckPoint(TaskInfo *md, TaskConfig *d, int check,
    int** coordsarr, MECHANIC_DATATYPE** resultarr){

  int i = 0;
  int mstat = 0;

  /* HDF */
  hid_t file_id, dset_board, dset_data, data_group;
  hid_t mapspace, memmapspace, rawspace, memrawspace;
  hsize_t co[2], rco[2];

  /* Open file */
  file_id = H5Fopen(d->datafile,H5F_ACC_RDWR,H5P_DEFAULT);

	mechanic_message(MECHANIC_MESSAGE_DEBUG,
      "Checkpoint file: %s\n", d->datafile);

	dset_board = H5Dopen(file_id, MECHANIC_DATABOARD, H5P_DEFAULT);
  data_group = H5Gopen(file_id, MECHANIC_DATAGROUP, H5P_DEFAULT);
  dset_data = H5Dopen(data_group, MECHANIC_DATASETMASTER, H5P_DEFAULT);

  /* We write pixels one by one */
  co[0] = 1;
  co[1] = 1;
  memmapspace = H5Screate_simple(MECHANIC_HDF_RANK, co, NULL);

  rco[0] = 1;
  rco[1] = md->output_length;
  memrawspace = H5Screate_simple(MECHANIC_HDF_RANK, rco, NULL);

  mapspace = H5Dget_space(dset_board);
  rawspace = H5Dget_space(dset_data);

  /* Write data */
  for (i = 0; i < check; i++) {

      /* Control board -- each computed pixel is marked with 1. */
      mstat = H5writeBoard(dset_board, memmapspace, mapspace, coordsarr[i]);

      /* Data */
      mstat = H5writeMaster(dset_data, memrawspace, rawspace, md, d,
          coordsarr[i], resultarr[i]);

  }

  H5Dclose(dset_board);
  H5Dclose(dset_data);
  H5Sclose(mapspace);
  H5Sclose(memmapspace);
  H5Sclose(rawspace);
  H5Sclose(memrawspace);
  H5Gclose(data_group);
  H5Fclose(file_id);

	mechanic_message(MECHANIC_MESSAGE_DEBUG, "Checkpoint finished\n");

  return mstat;

}

/*
 * Read simulation board
 *
 * FIX ME!
 * I couldn't manage to do it with single call to H5Dread and malloc,
 * so I select pixels one by one and read them to board array
 *
 * @todo return mstat instead of pixel. Return pixel in function arg
 */
int H5readBoard(TaskConfig* d, int** board, int *computed){

  hid_t file_id, dataset_id;
  hid_t dataspace_id, memspace_id;
  hsize_t co[2], offset[2], stride[2], block[2], dims[2];
  herr_t hdf_status = 0;
  int mstat = 0;

  int rdata[1][1];
  int i = 0, j = 0;
  
  /* Open checkpoint file */
  file_id = H5Fopen(d->datafile, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file_id < 0) return MECHANIC_ERR_HDF;

  dataset_id = H5Dopen(file_id, MECHANIC_DATABOARD, H5P_DEFAULT);
  if (dataset_id < 0) return MECHANIC_ERR_HDF;
  
  dataspace_id = H5Dget_space(dataset_id);
  if (dataspace_id < 0) return MECHANIC_ERR_HDF;

  /* Create memory space for one by one pixel read */
  dims[0] = 1;
  dims[1] = 1;
  memspace_id = H5Screate_simple(MECHANIC_HDF_RANK, dims, NULL);
  if (memspace_id < 0) return MECHANIC_ERR_HDF;

  /* Read board pixels one by one */
  for (i = 0; i < d->xres; i++) {
    for (j = 0; j < d->yres; j++) {

      offset[0] = i;
      offset[1] = j;

      co[0] = 1;
      co[1] = 1;

      stride[0] = 1;
      stride[1] = 1;

      block[0] = 1;
      block[1] = 1;

      rdata[0][0] = 0;

      hdf_status = H5Sselect_hyperslab(dataspace_id, H5S_SELECT_SET, offset,
          stride, co, block);
      if (hdf_status < 0) return MECHANIC_ERR_HDF;

      hdf_status = H5Dread(dataset_id, H5T_NATIVE_INT, memspace_id,
          dataspace_id, H5P_DEFAULT, rdata);
      if (hdf_status < 0) return MECHANIC_ERR_HDF;

      /* Copy temporary data array to board array */
      board[i][j] = rdata[0][0];
      if (board[i][j] == MECHANIC_TASK_FINISHED) *computed = *computed + 1;

    }
  }

  /* Close checkpoint file */
  H5Sclose(memspace_id);
  H5Sclose(dataspace_id);
  H5Dclose(dataset_id);
  H5Fclose(file_id);

  return mstat;
}

/* Validate checkpoint file */
int mechanic_validate_file(char* CheckpointFile) {
  int mstat = 0;
  hid_t file_id, dataset_id, datagroup_id;
  
  file_id = H5Fopen(CheckpointFile, H5F_ACC_RDONLY, H5P_DEFAULT);
  if (file_id < 0) return MECHANIC_ERR_CHECKPOINT;
  
  /* Check for Mechanic-specific datasets */
  
  // Board
  dataset_id = H5Dopen(file_id, MECHANIC_DATABOARD, H5P_DEFAULT);
  H5Dclose(dataset_id);

  // Master data group
  datagroup_id = H5Gopen(file_id, MECHANIC_DATAGROUP, H5P_DEFAULT);
  
  // Master dataset
  dataset_id = H5Dopen(datagroup_id, MECHANIC_DATASETMASTER, H5P_DEFAULT);
  H5Dclose(dataset_id);
  
  H5Gclose(datagroup_id);
  H5Fclose(file_id);

  return mstat;
}
