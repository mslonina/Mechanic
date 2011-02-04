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

/* [STORAGE] */

/**
 * @section storage Data Storage Scheme
 *
 * @M writes data in the following scheme:
 *
 * - @c /config -- configuration file (written by @c LRC API)
 * - @c /board -- simulation mapping
 * - @c /data -- main data group
 * - @c /data/master -- master dataset, contains data received from nodes.
 *
 * The file can be viewed with @c hdftools, i.e. @c h5dump. The master file
 * has always @c problemname-master.h5 name. If the master file exists in
 * the current working dir, it will be automatically backuped.
 *
 * The Parallel HDF has no support for MPI task farm, thus the only node
 * allowed to write master file is the master node. However, the module can
 * provide additional data files and operate on them, see @ref echo for
 * the example.
 */

/* [/STORAGE] */

/* [DATA] */

/**
 * @section showdata Working with Data
 * In this section you will find some tips and tricks of using data stored by @M.
 *
 * @subsection gnuplot Gnuplot
 * There are only three steps to prepare your data for Gnuplot:
 * 
 * 1. Dump data from HDF5 file:
 *  @code h5dump -d /data/master -y -w 100 -o output.dat mechanic-data.h5 @endcode
 * 2. Remove commas from @c output.dat:
 *  @code sed -s 's/,/ /g' output.dat @endcode
 * 3. For @c pm3d maps (200 is just your vertical resolution):
 *  @code sed "0~200G" output.dat > pm3d_file.dat @endcode
 *
 * You can process @c output.dat / @c pm3d_file.dat in the way you like.
 *
 */

/* [/DATA] */

/* HDF5 FUNCTIONS */

/* Master data scheme */
int H5createMasterDataScheme(hid_t file_id, moduleInfo *md, configData* d){

  hsize_t dimsf[2]; //dimsr[2];
  hid_t boardspace, dataspace;
  /*hid_t statsspace;*/
  hid_t dset_board, data_group, dset_data;
  /*hid_t dset_stats;*/
  herr_t hdf_status;
  int mstat = 0;

  mechanic_message(MECHANIC_MESSAGE_INFO, "Schema is: %d %d %d\n", 
    md->schema[0].rank, md->schema[0].dimsize[0], md->schema[0].dimsize[1]);
  mechanic_message(MECHANIC_MESSAGE_INFO, "Schema is: %d %d %d\n", 
    md->schema[1].rank, md->schema[1].dimsize[0], md->schema[1].dimsize[1]);

  /* Control board space */
  dimsf[0] = d->xres;
  dimsf[1] = d->yres;
  boardspace = H5Screate_simple(MECHANIC_HDF_RANK, dimsf, NULL);

  dset_board = H5Dcreate(file_id, MECHANIC_DATABOARD, H5T_NATIVE_INT,
    boardspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /* Master data group */
  data_group = H5Gcreate(file_id, MECHANIC_DATAGROUP,
    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /* Result data space */
  dataspace = H5Screate(md->schema[0].type);
  if (md->schema[0].type == H5S_SIMPLE) {
    hdf_status = H5Sset_extent_simple(dataspace, md->schema[0].rank, md->schema[0].dimsize, NULL);
  }

  /* Create master dataset */
  dset_data = H5Dcreate(data_group, md->schema[0].path, md->schema[0].datatype,
    dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  //dset_data = H5Dcreate(data_group, MECHANIC_DATASETMASTER, H5T_NATIVE_DOUBLE,
  //  dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  //dset_data = H5Dcreate(data_group, md->schema[1].path, H5T_NATIVE_DOUBLE,
  //  dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /* Create stats dataset */
  /*dset_stats = H5Dcreate(stats_group, MECHANIC_STATSMASTER, H5T_NATIVE_DOUBLE,
     dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  */
  H5Dclose(dset_board);
  H5Dclose(dset_data);
  H5Sclose(boardspace);
  H5Sclose(dataspace);
  H5Gclose(data_group);

  if (hdf_status < 0) mstat = MECHANIC_ERR_HDF;
  return mstat;
}

/* Write data to master file */
int H5writeMaster(hid_t dset, hid_t memspace, hid_t space, moduleInfo *md,
    configData* d, int* coordsarr, MECHANIC_DATATYPE* resultarr){

  MECHANIC_DATATYPE rdata[md->mrl][1]; /* And this is the place where C99 helps... */
  hsize_t co[2], off[2];
  herr_t hdf_status;
  int mstat = 0;
  int j = 0;

  co[0] = 1;
  co[1] = md->mrl;

  off[0] = coordsarr[2];
  off[1] = 0;

  for (j = 0; j < md->mrl; j++){
    rdata[j][0] = resultarr[j];
  }

  H5Sselect_hyperslab(space, H5S_SELECT_SET, off, NULL, co, NULL);
  hdf_status = H5Dwrite(dset, H5T_NATIVE_DOUBLE, memspace, space,
      H5P_DEFAULT, rdata);

  if (hdf_status < 0) mstat = MECHANIC_ERR_HDF;
  return mstat;
}

/* Mark computed pixels on board */
int H5writeBoard(hid_t dset, hid_t memspace, hid_t space, int* coordsarr){

  int rdata[1][1];
  hsize_t co[2], off[2];
  herr_t hdf_status;
  int mstat = 0;

  co[0] = 1;
  co[1] = 1;

  off[0] = coordsarr[0];
  off[1] = coordsarr[1];

  rdata[0][0] = MECHANIC_TASK_FINISHED;

  H5Sselect_hyperslab(space, H5S_SELECT_SET, off, NULL, co, NULL);
  hdf_status = H5Dwrite(dset, H5T_NATIVE_INT, memspace, space,
      H5P_DEFAULT, rdata);

  if (hdf_status < 0) mstat = MECHANIC_ERR_HDF;
  return mstat;
}

