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
#include "mechanic_module_echo.h"

/* [ECHO] */

/**
 * @section echo The Echo Module
 *
 * Here we present possible usage of the template system. We will use
 * @c node_in() and @c node_out() functions as examples.
 *
 * We implement @c node_in() and @c node_out() functions as follows:
 *
 * @code
 * @icode modules/echo/mechanic_module_echo.c ECHO_NODEIN
 * @endcode
 *
 * @code
 * @icode modules/echo/mechanic_module_echo.c ECHO_NODEOUT
 * @endcode
 *
 * They will be used if no override is present. However, we can create
 * overrides. For the master node we have:
 *
 * @code
 * @icode modules/echo/mechanic_module_echo.c ECHO_MASTERIN
 * @endcode
 *
 * which will override the output of @c node_in() on the master node. We can
 * create a much more complicated function, as for the @c node_in() at slave
 * node:
 *
 * @code
 * @icode modules/echo/mechanic_module_echo.c ECHO_SLAVEIN
 * @endcode
 *
 * This function use advantage of @c HDF storage. Each slave will create its
 * own data file and print a comment to it.
 *
 * Now, after all pixel have been computed, we tell our master node to copy
 * slave data files to the master data file, as shown below:
 *
 * @code
 * @icode modules/echo/mechanic_module_echo.c ECHO_MASTEROUT
 * @endcode
 *
 * At the end of simulation, the slave node will print customized message:
 * @code
 * @icode modules/echo/mechanic_module_echo.c ECHO_SLAVEOUT
 * @endcode
 */

/* [/ECHO] */

/**
 * Implementation of module_init()
 */
int echo_init(moduleInfo* md){

  md->name = "echo";
  md->author = "Mariusz Slonina";
  md->date = "2010";
  md->version = "1.0";
  md->mrl = 10;

  return 0;
}

/**
 * Implementation of module_query()
 */
int echo_query(moduleInfo* md){

  return 0;
}

/**
 * Implementation od module_cleanup()
 */
int echo_cleanup(moduleInfo* md){

  return 0;
}

/**
 * Implementation of module_farmResolution()
 */
int echo_farmResolution(int x, int y, moduleInfo* md, configData* d){

  int farm;

  farm = x*y;

  return farm;
}

/**
 * Implementation of module_pixelCoordsMap
 */
int echo_pixelCoordsMap(int t[], int p, int x, int y, moduleInfo* md,
    configData* d){

    if (p < y) {
      t[0] = p / y;
      t[1] = p;
    }

    if (p > y - 1) {
      t[0] = p / y;
      t[1] = p % y;
    }

  return 0;
}

/**
 * Implementation of module_pixelCoords()
 *
 * Default assignment is to copy t[] values to master result array:
 * @code
 * r->coords[0] = t[0] //x
 * r->coords[1] = t[1] //y
 * r->coords[2] = t[2] //number of the pixel
 * @endcode
 *
 */
int echo_pixelCoords(int node, int t[], moduleInfo* md, configData* d,
    masterData* r){

  r->coords[0] = t[0]; /* x */
  r->coords[1] = t[1]; /* y */
  r->coords[2] = t[2]; /* number of the pixel */

  return 0;
}

/**
 * Implementation of module_pixelCompute()
 */
int echo_pixelCompute(int node, moduleInfo* md, configData* d, masterData* r){

  int i = 0;

  for (i = 0; i < md->mrl; i++) {
    r->res[i] = (double) r->coords[2] * (double) i;
  }

  return 0;
}

/**
 * Implementation of module_node_beforePixelCompute()
 */
int echo_node_beforePixelCompute(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_afterPixelCompute()
 */
int echo_node_afterPixelCompute(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_in()
 */

/* [ECHO_NODEIN] */

int echo_node_in(int mpi_size, int node, moduleInfo* md, configData* d){

  mechanic_message(MECHANIC_MESSAGE_INFO, "NodeIN [%d]\n", node);

  return 0;
}

/* [/ECHO_NODEIN] */

/**
 * Implementation of module_node_out()
 */

/* [ECHO_NODEOUT] */

int echo_node_out(int mpi_size, int node, moduleInfo* md, configData* d,
    masterData* r){

  mechanic_message(MECHANIC_MESSAGE_INFO, "NodeOUT [%d]\n", node);

  return 0;
}

/* [/ECHO_NODEOUT] */

/**
 * Implementation of module_node_beforeSend()
 */
int echo_node_beforeSend(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_afterSend()
 */
int echo_node_afterSend(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_beforeReceive()
 */
int echo_node_beforeReceive(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_afterReceive()
 */
int echo_node_afterReceive(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_in()
 */

/* [ECHO_MASTERIN] */

int echo_master_in(int mpi_size, int node, moduleInfo* md, configData* d){

  return 0;
}

/* [/ECHO_MASTERIN] */

/**
 * Implementation of module_node_out()
 *
 * Example:
 * Here, we just copy slave data files into one master file.
 */

/* [ECHO_MASTEROUT] */
int echo_master_out(int nodes, int node, moduleInfo* md, configData* d,
    masterData* r){

  int i = 0;
  hid_t fname, masterfile, masterdatagroup;
  herr_t stat;
  char groupname[512];
  char filename[512];

  stat = H5open();

  mechanic_message(MECHANIC_MESSAGE_INFO, "ECHO MASTER IN: %s\n", d->datafile);
  masterfile = H5Fopen(d->datafile, H5F_ACC_RDWR, H5P_DEFAULT);
  masterdatagroup = H5Gopen(masterfile, "data", H5P_DEFAULT);

  /* Copy data from slaves to one master file */
  for (i = 1; i < nodes; i++) {
    sprintf(groupname, "slave%d", i);
    sprintf(filename, "%s-slave%d.h5", d->name, i);

    fname = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);
    stat = H5Ocopy(fname, groupname, masterdatagroup, groupname,
        H5P_DEFAULT, H5P_DEFAULT);
    if (stat < 0) mechanic_message(MECHANIC_MESSAGE_ERR, "copy error\n");

    H5Fclose(fname);

  }

  H5Gclose(masterdatagroup);
  H5Fclose(masterfile);
  stat = H5close();

  mechanic_message(MECHANIC_MESSAGE_INFO,
      "Master process [%d] OVER & OUT.\n", node);

  return 0;
}

/* [/ECHO_MASTEROUT] */

/**
 * Implementation of module_node_beforeSend()
 */
int echo_master_beforeSend(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_afterSend()
 */
int echo_master_afterSend(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_beforeReceive()
 */
int echo_master_beforeReceive(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_afterReceive()
 */
int echo_master_afterReceive(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_in()
 *
 * Example:
 * Here we create slave specific data file.
 * You can handle here any type of datasets etc.
 *
 * Data group is incorporated in MASTER_OUT function to one master data file.
 *
 */

/* [ECHO_SLAVEIN] */
int echo_slave_in(int mpi_size, int node, moduleInfo* md, configData* d,
    masterData* r){

  hid_t sfile_id, gid, string_type;
  hid_t dataset, dataspace;
  hid_t rank = 1;
  hsize_t dimens_1d;
  herr_t serr;
  char sbase[] = "slave";
  char nodename[512];
  char gbase[] = "slave";
  char group[512];
  char oldfile[1028];

  char cbase[] = "Hello from slave ";
  char comment[1024];

  struct stat st;

  mechanic_message(MECHANIC_MESSAGE_INFO, "ECHO IN: %s\n", d->name);
  sprintf(nodename, "%s-%s%d.h5", d->name, sbase, node);
  sprintf(group, "%s%d", gbase, node);

  /*
   * Imagine this:
   * each slave can create different dataspaces and datasets here, 
   * perform different computations, even read different config file!
   */
  if (stat(nodename,&st) == 0) {
      sprintf(oldfile, "old-%s", nodename);
      rename(nodename,oldfile);
  }

  sfile_id = H5Fcreate(nodename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  gid = H5Gcreate(sfile_id, group, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  sprintf(comment, "%s%d. ", cbase, node);
  string_type = H5Tcopy(H5T_C_S1);
  H5Tset_size(string_type, strlen(comment));

  rank = 1;
  dimens_1d = 1;

  dataspace = H5Screate_simple(rank, &dimens_1d, NULL);

  dataset = H5Dcreate(gid, "comment", string_type, dataspace, H5P_DEFAULT,
      H5P_DEFAULT, H5P_DEFAULT);
  serr = H5Dwrite(dataset, string_type, H5S_ALL, dataspace, H5P_DEFAULT,
      comment);

  H5Sclose(dataspace);
  H5Dclose(dataset);
  H5Gclose(gid);
  H5Fclose(sfile_id);

  return 0;
}

/* [/ECHO_SLAVEIN] */

/**
 * Implementation of module_node_out()
 */

/* [ECHO_SLAVEOUT] */

int echo_slave_out(int mpi_size, int node, moduleInfo* md, configData* d,
    masterData* r){

  mechanic_message(MECHANIC_MESSAGE_INFO, "SLAVE[%d] OVER & OUT\n", node);

  return 0;
}

/* [/ECHO_SLAVEOUT] */

/**
 * Implementation of module_node_beforeSend()
 */
int echo_slave_beforeSend(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_afterSend()
 */
int echo_slave_afterSend(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_beforeReceive()
 */
int echo_slave_beforeReceive(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

/**
 * Implementation of module_node_afterReceive()
 */
int echo_slave_afterReceive(int node, moduleInfo* md, configData* d,
    masterData* r){

  return 0;
}

