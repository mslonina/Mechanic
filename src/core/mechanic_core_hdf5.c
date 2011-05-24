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

  hsize_t dimsf[2];
  hid_t boardspace, dataspace;
  hid_t dset_board, data_group, dset_data;
  herr_t hdf_status;
  int mstat = 0;
  int i;

  /* Control board space */
  dimsf[0] = d->xres;
  dimsf[1] = d->yres;
  boardspace = H5Screate_simple(MECHANIC_HDF_RANK, dimsf, NULL);

  dset_board = H5Dcreate(file_id, MECHANIC_DATABOARD, H5T_NATIVE_INT,
    boardspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  mstat = H5createMasterAttributes(file_id);

  /* Master data group */
  data_group = H5Gcreate(file_id, MECHANIC_DATAGROUP,
    H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /* Create data schema */
  for (i = 0; i < md->schemasize; i++) {

    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Schema for %s is: %d %d %d\n", 
      md->schema[i].path, md->schema[i].rank, md->schema[i].dimsize[0], md->schema[i].dimsize[1]);
 
    dataspace = H5Screate(md->schema[i].type);
    if (md->schema[i].type == H5S_SIMPLE) {
      hdf_status = H5Sset_extent_simple(dataspace, md->schema[i].rank, md->schema[i].dimsize, NULL);
    }

    dset_data = H5Dcreate(data_group, md->schema[i].path, md->schema[i].datatype,
      dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  }
  
  H5Dclose(dset_board);
  H5Dclose(dset_data);
  H5Sclose(boardspace);
  H5Sclose(dataspace);
  H5Gclose(data_group);

  if (hdf_status < 0) mstat = MECHANIC_ERR_HDF;
  return mstat;
}

int H5createMasterAttributes(hid_t loc_id) {

  int mstat = 0;
  hid_t aspace_id;
  hsize_t attr_dims;
  hid_t attr_software_id, attr_majorv_id, attr_minorv_id, attr_patchv_id, attr_sapiv_id, attr_mapiv_id;
  hid_t attr_software_t, attr_majorv_t, attr_minorv_t, attr_patchv_t, attr_sapiv_t, attr_mapiv_t;
  hid_t attr_software_mt, attr_majorv_mt, attr_minorv_mt, attr_patchv_mt, attr_sapiv_mt, attr_mapiv_mt;
  size_t len;

  /* Attach global attributes */
  attr_dims = 1;

  len = strlen(MECHANIC_NAME)+1;
  attr_software_t = H5Tcopy(H5T_C_S1); H5Tset_size(attr_software_t, len);
  attr_software_mt = H5Tcopy(H5T_C_S1); H5Tset_size(attr_software_mt, len);
  
  len = strlen(MECHANIC_VERSION_MAJOR)+1;
  attr_majorv_t = H5Tcopy(H5T_C_S1); H5Tset_size(attr_majorv_t, len);
  attr_majorv_mt = H5Tcopy(H5T_C_S1); H5Tset_size(attr_majorv_mt, len);

  len = strlen(MECHANIC_VERSION_MINOR)+1;
  attr_minorv_t = H5Tcopy(H5T_C_S1); H5Tset_size(attr_minorv_t, len);
  attr_minorv_mt = H5Tcopy(H5T_C_S1); H5Tset_size(attr_minorv_mt, len);

  len = strlen(MECHANIC_VERSION_PATCH)+1;
  attr_patchv_t = H5Tcopy(H5T_C_S1); H5Tset_size(attr_patchv_t, len);
  attr_patchv_mt = H5Tcopy(H5T_C_S1); H5Tset_size(attr_patchv_mt, len);

  len = strlen(MECHANIC_STORAGE_API)+1;
  attr_sapiv_t = H5Tcopy(H5T_C_S1); H5Tset_size(attr_sapiv_t, len);
  attr_sapiv_mt = H5Tcopy(H5T_C_S1); H5Tset_size(attr_sapiv_mt, len);
  
  len = strlen(MECHANIC_MODULE_API_C)+1;
  attr_mapiv_t = H5Tcopy(H5T_C_S1); H5Tset_size(attr_mapiv_t, len);
  attr_mapiv_mt = H5Tcopy(H5T_C_S1); H5Tset_size(attr_mapiv_mt, len);

  aspace_id = H5Screate_simple(1,&attr_dims,NULL);

  attr_software_id = H5Acreate(loc_id, "Software", attr_software_t, aspace_id, H5P_DEFAULT,H5P_DEFAULT);
  attr_majorv_id = H5Acreate(loc_id, "Major Version", attr_majorv_t, aspace_id, H5P_DEFAULT,H5P_DEFAULT);
  attr_minorv_id = H5Acreate(loc_id, "Minor Version", attr_minorv_t, aspace_id, H5P_DEFAULT,H5P_DEFAULT);
  attr_patchv_id = H5Acreate(loc_id, "Patch Version", attr_patchv_t, aspace_id, H5P_DEFAULT,H5P_DEFAULT);
  attr_sapiv_id = H5Acreate(loc_id, "Storage API Version", attr_sapiv_t, aspace_id, H5P_DEFAULT,H5P_DEFAULT);
  attr_mapiv_id = H5Acreate(loc_id, "Module API Version", attr_mapiv_t, aspace_id, H5P_DEFAULT,H5P_DEFAULT);
  
  H5Awrite(attr_software_id, attr_software_mt, MECHANIC_NAME);
  H5Awrite(attr_majorv_id, attr_majorv_mt, MECHANIC_VERSION_MAJOR);
  H5Awrite(attr_minorv_id, attr_minorv_mt, MECHANIC_VERSION_MINOR);
  H5Awrite(attr_patchv_id, attr_patchv_mt, MECHANIC_VERSION_PATCH);
  H5Awrite(attr_sapiv_id, attr_sapiv_mt, MECHANIC_STORAGE_API);
  H5Awrite(attr_mapiv_id, attr_mapiv_mt, MECHANIC_MODULE_API_C);
  
  H5Aclose(attr_software_id); H5Tclose(attr_software_mt); H5Tclose(attr_software_t);
  H5Aclose(attr_majorv_id); H5Tclose(attr_majorv_mt); H5Tclose(attr_majorv_t);
  H5Aclose(attr_minorv_id); H5Tclose(attr_minorv_mt); H5Tclose(attr_minorv_t);
  H5Aclose(attr_patchv_id); H5Tclose(attr_patchv_mt); H5Tclose(attr_patchv_t);
  H5Aclose(attr_sapiv_id); H5Tclose(attr_sapiv_mt); H5Tclose(attr_sapiv_t);
  H5Aclose(attr_mapiv_id); H5Tclose(attr_mapiv_mt); H5Tclose(attr_mapiv_t);

  H5Sclose(aspace_id);

  return mstat;
}

/* Write data to master file */
int H5writeMaster(hid_t dset, hid_t memspace, hid_t space, moduleInfo *md,
    configData* d, int* coordsarr, MECHANIC_DATATYPE* resultarr){

  //MECHANIC_DATATYPE rdata[md->mrl][1]; /* And this is the place where C99 helps... */
  //MECHANIC_DATATYPE **rdata;
  hsize_t co[2], off[2];
  herr_t hdf_status;
  int mstat = 0;
  //int j = 0;

  /* Allocate memory for rdata */
  //rdata = AllocateDouble2D(md->mrl, 1);

  co[0] = 1;
  co[1] = md->mrl;

  off[0] = coordsarr[2];
  off[1] = 0;

  /*for (j = 0; j < md->mrl; j++){
    rdata[j][0] = resultarr[j];
    printf("%.2f ", rdata[j][0]);
  }
  printf("\n");
*/
  H5Sselect_hyperslab(space, H5S_SELECT_SET, off, NULL, co, NULL);
  hdf_status = H5Dwrite(dset, H5T_NATIVE_DOUBLE, memspace, space,
    //  H5P_DEFAULT, *rdata[]);
      H5P_DEFAULT, resultarr);

  //FreeDouble2D(rdata, md->mrl);

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

