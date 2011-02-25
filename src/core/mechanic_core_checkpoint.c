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

int atCheckPoint(int check, int** coordsarr, int** board,
    MECHANIC_DATATYPE** resultarr, moduleInfo* md, configData* d){

  int mstat = 0;

  mstat = manageCheckPoints(d);
  mstat = H5writeCheckPoint(md, d, check, coordsarr, resultarr);

  if (mstat < 0) {
    mechanic_message(MECHANIC_MESSAGE_ERR, "Checkpoint failed, aborting\n");
  }

  return mstat;
}

int manageCheckPoints(configData* d){

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
  if (checkpoint == NULL) mechanic_error(MECHANIC_ERR_MEM);

  checkpoint_old = calloc(clen, sizeof(char*));
  if (checkpoint_old == NULL) mechanic_error(MECHANIC_ERR_MEM);

  for (i = MECHANIC_CHECKPOINTS-2; i >= 0; i--) {

    snprintf(checkpoint, clen, "%s-%s-%02d.%s",
        d->name, MECHANIC_MASTER_PREFIX_DEFAULT, i+1, MECHANIC_FILE_EXT);
    snprintf(checkpoint_old, clen, "%s-%s-%02d.%s",
        d->name, MECHANIC_MASTER_PREFIX_DEFAULT, i, MECHANIC_FILE_EXT);

    if (stat(checkpoint_old, &cto) == 0 ) {
      if (stat(checkpoint, &ct) < 0) {

        /* If the checkpoint file doesn't exist, copy current file */
        mstat = mechanic_copy(checkpoint_old, checkpoint);
        if (mstat < 0) mechanic_abort(MECHANIC_ERR_CHECKPOINT);

      } else {

        /* Instead of renaming, if we reach 00 file, we copy it to 01 */
        if (i == 0) {
          mstat = mechanic_copy(checkpoint_old, checkpoint);
          if (mstat < 0) mechanic_abort(MECHANIC_ERR_CHECKPOINT);
        } else {
          mstat = rename(checkpoint_old, checkpoint);
          if (mstat < 0) mechanic_abort(MECHANIC_ERR_CHECKPOINT);
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
int H5writeCheckPoint(moduleInfo *md, configData *d, int check,
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
  rco[1] = md->mrl;
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
int H5readBoard(configData* d, int** board){

  hid_t file_id, dataset_id;
  hid_t dataspace_id, memspace_id;
  hsize_t co[2], offset[2], stride[2], block[2], dims[2];
  herr_t hdf_status;
  int mstat = 0;

  int rdata[1][1];
  int i = 0, j = 0;
  int computed = 0;

  /* Open checkpoint file */
  file_id = H5Fopen(d->datafile, H5F_ACC_RDONLY, H5P_DEFAULT);
  dataset_id = H5Dopen(file_id, MECHANIC_DATABOARD, H5P_DEFAULT);
  dataspace_id = H5Dget_space(dataset_id);

  /* Create memory space for one by one pixel read */
  dims[0] = 1;
  dims[1] = 1;
  memspace_id = H5Screate_simple(MECHANIC_HDF_RANK, dims, NULL);

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

      hdf_status = H5Dread(dataset_id, H5T_NATIVE_INT, memspace_id,
          dataspace_id, H5P_DEFAULT, rdata);

      /* Copy temporary data array to board array */
      board[i][j] = rdata[0][0];
      if (board[i][j] == 1) computed++;

    }
  }

  /* Close checkpoint file */
  H5Sclose(memspace_id);
  H5Sclose(dataspace_id);
  H5Dclose(dataset_id);
  H5Fclose(file_id);

  return computed;
}

