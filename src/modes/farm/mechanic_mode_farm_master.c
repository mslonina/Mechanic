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
#include "mechanic_mode_farm.h"

/* MASTER */
int mechanic_mode_farm_master(int mpi_size, int node, mechanic_internals handler,
    moduleInfo* md, configData* d){

  int i = 0, j = 0, nodes = 0, farm_res = 0;
  int npxc = 0; /* number of all sent pixels */
  int count = 0; /* number of pixels to receive */
  int check = 0; /* current number of received data */

  int mstat;
  int ptab[3];

  masterData result;
  masterData inidata;

  /* Checkpoint storage */
  int vecsize;
  int* coordsvec;
  int** coordsarr;
  MECHANIC_DATATYPE *resultsvec;
  MECHANIC_DATATYPE **resultarr;

  /* Simulation board */
  int** board;
  int computed = 0; /* How many pixels have been computed */
  int pixeldiff = 0; /* Difference between farm resolution and number of computed pixels*/
  int totalnumofpx = 0; /* Total number of pixels received */

  MPI_Status mpi_status;
  MPI_Datatype masterResultsType;
  MPI_Datatype initialConditionsType;

  module_query_int_f query;

  /* Allocate memory */
  result.res = AllocateDoubleVec(md->mrl);
  inidata.res = AllocateDoubleVec(md->irl);

  coordsarr = AllocateInt2D(d->checkpoint,3);
  resultarr = AllocateDouble2D(d->checkpoint,md->mrl);

  board = AllocateInt2D(d->xres,d->yres);

  vecsize = 3;

  /* For the sake of simplicity we read board everytime,
   * both in restart and clean simulation mode */
  computed = H5readBoard(d, board);
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Num of computed pixels = %d", computed);

  /* Build derived type for master result */
  mstat = buildMasterResultsType(md->mrl, &result, &masterResultsType);
  mechanic_check_mstat(mstat);

  mstat = buildMasterResultsType(md->irl, &inidata, &initialConditionsType);
  mechanic_check_mstat(mstat);

  /* Master can do something useful before computations. */
  query = mechanic_load_sym(handler, "in", MECHANIC_MODULE_SILENT);
  if (query) mstat = query(mpi_size, node, md, d, &inidata);
  mechanic_check_mstat(mstat);

  /* Align farm resolution for given method. */
  query = mechanic_load_sym(handler, "farmResolution", MECHANIC_MODULE_ERROR);
  if (query) farm_res = query(d->xres, d->yres, md, d);
  if (farm_res > (d->xres * d->yres)) {
    mechanic_message(MECHANIC_MESSAGE_ERR,
      "Farm resolution should not exceed x*y!\n");
    mechanic_error(MECHANIC_ERR_SETUP);
  }

  /* FARM STARTS */

  /* Security check -- if farm resolution is greater than number of slaves.
   * Needed when we have i.e. 3 slaves and only one pixel to compute. */
  pixeldiff = farm_res - computed;
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "farm_res - computed = %d\n",
    pixeldiff);
  if (pixeldiff > mpi_size) nodes = mpi_size;
  if (pixeldiff == mpi_size) nodes = mpi_size;
  if (pixeldiff < mpi_size) nodes = pixeldiff + 1;

  /* Send tasks to all slaves,
   * remembering what the farm resolution is.*/
  for (i = 1; i < nodes; i++) {

    npxc = map2d(npxc, handler, md, d, ptab, board);
    count++;

    inidata.coords[0] = ptab[0];
    inidata.coords[1] = ptab[1];
    inidata.coords[2] = ptab[2];

    query = mechanic_load_sym(handler, "preparePixel", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(node, md, d, &inidata, &result);
    mechanic_check_mstat(mstat);

    mechanic_message(MECHANIC_MESSAGE_CONT,
        "Pixel [%04d, %04d, %04d] sended to node %04d\n",
        ptab[0], ptab[1], ptab[2], i);

    query = mechanic_load_sym(handler, "beforeSend", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(i, md, d, &result);
    mechanic_check_mstat(mstat);

#ifdef IPM
    MPI_Pcontrol(1, "master_ini_send");
#endif

    MPI_Send(&inidata, 1, initialConditionsType, i,
        MECHANIC_MPI_DATA_TAG, MPI_COMM_WORLD);

#ifdef IPM
    MPI_Pcontrol(-1, "master_ini_send");
#endif

    query = mechanic_load_sym(handler, "afterSend", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(i, md, d, &result);
    mechanic_check_mstat(mstat);

  }

  /* We don't want to have slaves idle, so, if there are some idle slaves
   * terminate them (when farm resolution < mpi size). */
  if ((farm_res - computed) < mpi_size) {
    for (i = nodes; i < mpi_size; i++) {

      mechanic_message(MECHANIC_MESSAGE_WARN,
          "Terminating idle slave %d.\n", i);

      /* Just dummy assignment */
      inidata.coords[0] = inidata.coords[1] = inidata.coords[2] = 0;

      MPI_Send(&inidata, 1, initialConditionsType, i,
          MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);
    }
  }

  /* Receive data and send tasks. */
  while (1) {

    query = mechanic_load_sym(handler, "beforeReceive", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(0, md, d, &inidata, &result);

#ifdef IPM
    MPI_Pcontrol(1,"master_result_recv");
#endif

    MPI_Recv(&result, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG,
        MPI_COMM_WORLD, &mpi_status);

#ifdef IPM
    MPI_Pcontrol(-1,"master_result_recv");
#endif

    totalnumofpx++;
    count--;

    query = mechanic_load_sym(handler, "afterReceive", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(mpi_status.MPI_SOURCE, md, d, &inidata, &result);
    mechanic_check_mstat(mstat);

    /* Copy data to checkpoint arrays */
    coordsarr[check][0] = result.coords[0];
    coordsarr[check][1] = result.coords[1];
    coordsarr[check][2] = result.coords[2];

    mechanic_message(MECHANIC_MESSAGE_CONT,
        "[%04d / %04d] Pixel [%04d, %04d, %04d] received from node %d\n",
        totalnumofpx, pixeldiff,  result.coords[0], result.coords[1],
        result.coords[2], mpi_status.MPI_SOURCE);

    for (j = 0; j < md->mrl; j++) {
      resultarr[check][j] = result.res[j];
			mechanic_message(MECHANIC_MESSAGE_DEBUG, "%2.2f\t", result.res[j]);
    }

		mechanic_message(MECHANIC_MESSAGE_DEBUG, "\n");

    /* We write data only on checkpoint:
     * it is more healthier for disk, but remember --
     * if you set up checkpoints very often and your simulations are
     * very fast, your disks will not be happy
     */

    if ((((check+1) % d->checkpoint) == 0) || mechanic_ups() < 0) {

      /* Fortran interoperability:
       * Convert 2D coordinates array to 1D vector, as well as
       * results array */
      coordsvec = IntArrayToVec(coordsarr, d->checkpoint, vecsize);
      resultsvec = DoubleArrayToVec(resultarr, d->checkpoint, md->mrl);

      query = mechanic_load_sym(handler, "beforeCheckpoint", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(nodes, md, d, coordsvec, resultsvec);
      mechanic_check_mstat(mstat);

      mstat = atCheckPoint(check+1, coordsarr, board, resultarr, md, d);
      mechanic_check_mstat(mstat);
      check = 0;

      query = mechanic_load_sym(handler, "afterCheckpoint", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(nodes, md, d, coordsvec, resultsvec);
      mechanic_check_mstat(mstat);

      FreeIntVec(coordsvec);
      FreeDoubleVec(resultsvec);

    } else {
      check++;
    }

    /* Send next task to the slave */
    if (npxc < farm_res){

      npxc = map2d(npxc, handler, md, d, ptab, board);
      count++;

      inidata.coords[0] = ptab[0];
      inidata.coords[1] = ptab[1];
      inidata.coords[2] = ptab[2];

      query = mechanic_load_sym(handler, "preparePixel", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(node, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

      query = mechanic_load_sym(handler, "beforeSend", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(mpi_status.MPI_SOURCE, md, d, &inidata, &result);

      mechanic_message(MECHANIC_MESSAGE_CONT2,
        "Pixel [%04d, %04d, %04d] sended to node %d\n",
          inidata.coords[0], inidata.coords[1], inidata.coords[2], mpi_status.MPI_SOURCE);

#ifdef IPM
      MPI_Pcontrol(1,"master_pixel_send");
#endif

      MPI_Send(&inidata, 1, initialConditionsType, mpi_status.MPI_SOURCE,
          MECHANIC_MPI_DATA_TAG, MPI_COMM_WORLD);

#ifdef IPM
      MPI_Pcontrol(-1,"master_pixel_send");
#endif

      query = mechanic_load_sym(handler, "afterSend", MECHANIC_MODULE_SILENT);

      if(query) mstat = query(mpi_status.MPI_SOURCE, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

    } else {
      break;
    }
  } /* while(1) ends */

  /*
   * No more work to do, receive the outstanding results from the slaves.
   * We've got exactly 'count' messages to receive.
   */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Count is %d, Check is %d\n", count, check);
  for (i = 0; i < count; i++){

    query = mechanic_load_sym(handler, "beforeReceive", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(0, md, d, &inidata, &result);
    mechanic_check_mstat(mstat);

    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Recv... ");

#ifdef IPM
    MPI_Pcontrol(1,"master_final_recv");
#endif

    MPI_Recv(&result, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG,
        MPI_COMM_WORLD, &mpi_status);

#ifdef IPM
    MPI_Pcontrol(-1,"master_final_recv");
#endif

    mechanic_message(MECHANIC_MESSAGE_DEBUG, "done\n");
    totalnumofpx++;

    query = mechanic_load_sym(handler, "afterReceive", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(mpi_status.MPI_SOURCE, md, d, &inidata, &result);
    mechanic_check_mstat(mstat);

    /* Copy data to checkpoint arrays */
    coordsarr[check][0] = result.coords[0];
    coordsarr[check][1] = result.coords[1];
    coordsarr[check][2] = result.coords[2];

    for (j = 0; j < md->mrl; j++) {
      resultarr[check][j] = result.res[j];
    }

    mechanic_message(MECHANIC_MESSAGE_CONT,
        "[%04d / %04d] Pixel [%04d, %04d, %04d] received from node %d\n",
        totalnumofpx, pixeldiff,  result.coords[0], result.coords[1],
        result.coords[2], mpi_status.MPI_SOURCE);
    /* There is a possibility that count > check, and then we have to perform
     * another checkpoint write */
    if ((((check+1) % d->checkpoint) == 0) || mechanic_ups() < 0) {

      /* Fortran interoperability:
       * Convert 2D coordinates array to 1D vector, as well as
       * results array */
      coordsvec = IntArrayToVec(coordsarr, d->checkpoint, vecsize);
      resultsvec = DoubleArrayToVec(resultarr, d->checkpoint, md->mrl);

      query = mechanic_load_sym(handler, "beforeCheckpoint", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(nodes, md, d, coordsvec, resultsvec);
      mechanic_check_mstat(mstat);

      mstat = atCheckPoint(check+1, coordsarr, board, resultarr, md, d);
      mechanic_check_mstat(mstat);
      check = 0;

      query = mechanic_load_sym(handler, "afterCheckpoint", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(nodes, md, d, coordsvec, resultsvec);
      mechanic_check_mstat(mstat);

      if(coordsvec) free(coordsvec);
      if(resultsvec) free(resultsvec);

    } else {
      check++;
    }

  }

  /* Write outstanding results to file
   * If farm_res is smaller than checkpoint interval, it means, we write
   * all data at once, and we have to handle before/afterCheckpoint here
   * as well
   */

  /* Fortran interoperability:
   * Convert 2D coordinates array to 1D vector, as well as
   * results array */
  //coordsvec = IntArrayToVec(coordsarr, d->checkpoint, vecsize);
  //resultsvec = DoubleArrayToVec(resultarr, d->checkpoint, md->mrl);

  //query = mechanic_load_sym(handler, "node_beforeCheckpoint",
  //  "master_beforeCheckpoint", MECHANIC_MODULE_SILENT);
  //if (query) mstat = query(nodes, md, d, coordsvec, resultsvec);

  mstat = atCheckPoint(check, coordsarr, board, resultarr, md, d);
  mechanic_check_mstat(mstat);

  //query = mechanic_load_sym(handler, "node_afterCheckpoint",
  //    "master_afterCheckpoint", MECHANIC_MODULE_SILENT);
  //if (query) mstat = query(nodes, md, d, coordsvec, resultsvec);

  //if(coordsvec) free(coordsvec);
  //if(resultsvec) free(resultsvec);

  /* Now, terminate the slaves */
  for (i = 1; i < nodes; i++) {

     inidata.coords[0] = inidata.coords[1] = inidata.coords[2] = 0;
     MPI_Send(&inidata, 1, initialConditionsType, i,
         MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);

  }

  /* FARM ENDS */
  MPI_Type_free(&masterResultsType);
  MPI_Type_free(&initialConditionsType);

  /* Master can do something useful after the computations. */
  query = mechanic_load_sym(handler, "out", MECHANIC_MODULE_SILENT);
  if (query) mstat = query(nodes, node, md, d, &inidata, &result);
  mechanic_check_mstat(mstat);

  /* Release resources */
  FreeDoubleVec(result.res);
  FreeDoubleVec(inidata.res);
  FreeInt2D(coordsarr,d->checkpoint);
  FreeDouble2D(resultarr,d->checkpoint);
  FreeInt2D(board,d->xres);

  return mstat;
}

