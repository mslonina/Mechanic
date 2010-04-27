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
#include "mechanic_mode_farm.h"

/* MASTER */
int mechanic_mode_farm_master(int node, void* handler, moduleInfo* md,
    configData* d){

  int i = 0, j = 0, nodes = 0, farm_res = 0;
  int npxc = 0; /* number of all sent pixels */
  int count = 0; /* number of pixels to receive */
  int check = 0; /* current number of received data */

  int mstat;
  int ptab[3];

  masterData rawdata;

  /* Checkpoint storage */
  int** coordsarr;
  MECHANIC_DATATYPE **resultarr;

  /* Restart mode board */
  int** board;

  MPI_Status mpi_status;
  MPI_Datatype masterResultsType;

  module_query_void_f qbeforeS, qafterS, qbeforeR, qafterR;
  module_query_int_f qr;

  /* Allocate memory for rawdata.res array. */
  rawdata.res = realloc(NULL,
      ((uintptr_t) md->mrl) * sizeof(MECHANIC_DATATYPE));

  if (rawdata.res == NULL) mechanic_error(MECHANIC_ERR_MEM);

  coordsarr = calloc(sizeof(uintptr_t) * ((uintptr_t) d->checkpoint + 1), sizeof(uintptr_t));
  if (coordsarr == NULL) mechanic_error(MECHANIC_ERR_MEM);

  resultarr = calloc(sizeof(MECHANIC_DATATYPE) * ((uintptr_t) d->checkpoint + 1), sizeof(uintptr_t));
  if (resultarr == NULL) mechanic_error(MECHANIC_ERR_MEM);

  for (i = 0; i < d->checkpoint; i++) {
    coordsarr[i] = calloc(sizeof(uintptr_t) * 3, sizeof(uintptr_t));
    if (coordsarr[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);

    resultarr[i] = calloc(sizeof(MECHANIC_DATATYPE) * ((uintptr_t) md->mrl), sizeof(uintptr_t));
    if (resultarr[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);

  }

  /* Allocate memory for board */
  if (d->restartmode == 1) {
    board = calloc(sizeof(uintptr_t) * ((uintptr_t) d->xres), sizeof(uintptr_t));
    if (board == NULL) mechanic_error(MECHANIC_ERR_MEM);

    for (i = 0; i < d->xres; i++) {
      board[i] = calloc(sizeof(uintptr_t)*((uintptr_t)d->yres), sizeof(uintptr_t));
      if(board[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);
    }
  }

  if(d->restartmode == 1) H5readBoard(d, board);

  /* Build derived type for master result */
  mstat = buildMasterResultsType(md->mrl, &rawdata, &masterResultsType);

  /* Master can do something useful before computations. */
  query = load_sym(handler, d->module, "node_in", "master_in", MECHANIC_MODULE_SILENT);
  if (query) mstat = query(mpi_size, node, md, d);

  /* Align farm resolution for given method. */
  if (d->method == 0) farm_res = d->xres * d->yres;
  if (d->method == 6) {
    qr = load_sym(handler, d->module, "farmResolution", "farmResolution",
        MECHANIC_MODULE_ERROR);
    if (qr) farm_res = qr(d->xres, d->yres, md);
  }

  /* FARM STARTS */

  /* Security check -- if farm resolution is greater than number of slaves.
   * Needed when we have i.e. 3 slaves and only one pixel to compute. */
  if (farm_res > mpi_size) nodes = mpi_size;
  if (farm_res == mpi_size) nodes = mpi_size;
  if (farm_res < mpi_size) nodes = farm_res + 1;

  /* Send tasks to all slaves,
   * remembering what the farm resolution is.*/
  for (i = 1; i < nodes; i++) {

    qbeforeS = load_sym(handler, d->module, "node_beforeSend", "master_beforeSend",
        MECHANIC_MODULE_SILENT);
    if (qbeforeS) mstat = qbeforeS(i, md, d, &rawdata);

    map2d(npxc, handler, md, d, ptab);

    mechanic_message(MECHANIC_MESSAGE_DEBUG,
        "MASTER PTAB[%d, %d, %d\n]", ptab[0], ptab[1], ptab[2]);

    MPI_Send(ptab, 3, MPI_INT, i, MECHANIC_MPI_DATA_TAG, MPI_COMM_WORLD);

    qafterS = load_sym(handler, d->module, "node_afterSend", "master_afterSend",
        MECHANIC_MODULE_SILENT);
    if (qafterS) mstat = qafterS(i, md, d, &rawdata);

    count++;
    npxc++;
  }

  /* We don't want to have slaves idle, so, if there are some idle slaves
   * terminate them (when farm resolution < mpi size). */
  if (farm_res < mpi_size) {
    for (i = nodes; i < mpi_size; i++) {

      mechanic_message(MECHANIC_MESSAGE_WARN,
          "Terminating idle slave %d.\n", i);

      map2d(npxc, handler, md, d, ptab);

      mechanic_message(MECHANIC_MESSAGE_DEBUG,
          "MASTER PTAB[%d, %d, %d\n]", ptab[0], ptab[1], ptab[2]);

      MPI_Send(ptab, 3, MPI_INT, i, MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);
    }
  }

  /* Receive data and send tasks. */
  while (1) {

    qbeforeR = load_sym(handler, d->module, "node_beforeReceive",
        "master_beforeReceive", MECHANIC_MODULE_SILENT);
    if (qbeforeR) mstat = qbeforeR(0, md, d, &rawdata);

    MPI_Recv(&rawdata, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG,
        MPI_COMM_WORLD, &mpi_status);

    count--;

    qafterR = load_sym(handler, d->module, "node_afterReceive", "master_afterReceive",
        MECHANIC_MODULE_SILENT);
    if (qafterR) mstat = qafterR(mpi_status.MPI_SOURCE, md, d, &rawdata);

    /* Copy data to checkpoint arrays */
    coordsarr[check][0] = rawdata.coords[0];
    coordsarr[check][1] = rawdata.coords[1];
    coordsarr[check][2] = rawdata.coords[2];

    mechanic_message(MECHANIC_MESSAGE_DEBUG, "MASTER RECV[%d, %d, %d]\t",
        node, rawdata.coords[0], rawdata.coords[1], rawdata.coords[2]);

    for (j = 0; j < md->mrl; j++) {
      resultarr[check][j] = rawdata.res[j];
			mechanic_message(MECHANIC_MESSAGE_DEBUG, "%2.2f\t", rawdata.res[j]);
    }

		mechanic_message(MECHANIC_MESSAGE_DEBUG, "\n");

    check++;

    /* We write data only on checkpoint:
     * it is more healthier for disk, but remember --
     * if you set up checkpoints very often and your simulations are
     * very fast, your disks will not be happy
     */

    if (check % d->checkpoint == 0) { /* FIX ME: add UPS checks */
      mstat = atCheckPoint(check, coordsarr, board, resultarr, md, d);
      check = 0;
    }

    /* Send next task to the slave */
    if (npxc < farm_res){

      qbeforeS = load_sym(handler, d->module, "node_beforeSend", "master_beforeSend",
          MECHANIC_MODULE_SILENT);
      if (qbeforeS) mstat = qbeforeS(mpi_status.MPI_SOURCE, md, d, &rawdata);

      map2d(npxc, handler, md, d, ptab);

      mechanic_message(MECHANIC_MESSAGE_DEBUG, "MASTER PTAB[%d, %d, %d\n]",
          ptab[0], ptab[1], ptab[2]);

      MPI_Send(ptab, 3, MPI_INT, mpi_status.MPI_SOURCE, MECHANIC_MPI_DATA_TAG,
          MPI_COMM_WORLD);

      npxc++;
      count++;

      qafterS = load_sym(handler, d->module, "node_afterSend", "master_afterSend",
          MECHANIC_MODULE_SILENT);

      if(qafterS) mstat = qafterS(mpi_status.MPI_SOURCE, md, d, &rawdata);

    } else {
      break;
    }
  } /* while(1) ends */

  /*
   * No more work to do, receive the outstanding results from the slaves.
   * We've got exactly 'count' messages to receive.
   */
  for (i = 0; i < count; i++){

    qbeforeR = load_sym(handler, d->module, "node_beforeReceive",
        "master_beforeReceive", MECHANIC_MODULE_SILENT);
    if (qbeforeR) mstat = qbeforeR(0, md, d, &rawdata);

    MPI_Recv(&rawdata, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG,
        MPI_COMM_WORLD, &mpi_status);

    qafterR = load_sym(handler, d->module, "node_afterReceive", "master_afterReceive",
        MECHANIC_MODULE_SILENT);
    if (qafterR) mstat = qafterR(mpi_status.MPI_SOURCE, md, d, &rawdata);

    /* Copy data to checkpoint arrays */
    coordsarr[check][0] = rawdata.coords[0];
    coordsarr[check][1] = rawdata.coords[1];
    coordsarr[check][2] = rawdata.coords[2];

    for (j = 0; j < md->mrl; j++) {
      resultarr[check][j] = rawdata.res[j];
    }

    check++;
  }

  /* Write outstanding results to file */
  mstat = atCheckPoint(check, coordsarr, board, resultarr, md, d);

  /* Now, terminate the slaves */
  for (i = 1; i < nodes; i++) {

     map2d(npxc, handler, md, d, ptab);

     mechanic_message(MECHANIC_MESSAGE_DEBUG,
         "MASTER PTAB[%d, %d, %d\n]", ptab[0], ptab[1], ptab[2]);

     MPI_Send(ptab, 3, MPI_INT, i, MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);

  }

  /* FARM ENDS */
  MPI_Type_free(&masterResultsType);

  /* Master can do something useful after the computations. */
  query = load_sym(handler, d->module, "node_out", "master_out",
      MECHANIC_MODULE_SILENT);
  if (query) mstat = query(nodes, node, md, d, &rawdata);

  free(rawdata.res);

  for (i = 0; i < d->checkpoint; i++) {
    free(coordsarr[i]);
    free(resultarr[i]);
  }

  free(coordsarr);
  free(resultarr);

  if (d->restartmode == 1) {
    for (i = 0; i < d->xres; i++) free(board[i]);
    free(board);
  }

  return 0;
}

