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
int mechanic_mode_farm_master(mechanic_internals *handler) {

  int i = 0, j = 0, farm_res = 0;
  int npxc = 0; /* number of all sent pixels */
  int count = 0; /* number of pixels to receive */
  int check = 0; /* current number of received data */

  int mstat;
  int ptab[3];

  TaskData result;
  TaskData inidata;

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
  result.data = AllocateDoubleVec(handler->info->output_length);
  inidata.data = AllocateDoubleVec(handler->info->input_length);

  coordsarr = AllocateInt2D(handler->config->checkpoint,3);
  resultarr = AllocateDouble2D(handler->config->checkpoint,handler->info->output_length);

  board = AllocateInt2D(handler->config->xres,handler->config->yres);

  vecsize = 3;

  /* For the sake of simplicity we read board everytime,
   * both in restart and clean simulation mode */
  mstat = H5readBoard(handler->config, board, &computed);
  mechanic_check_mstat(mstat);
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Num of computed pixels = %d\n", computed);

  /* Build derived type for master result */
  mstat = buildMasterResultsType(handler->info->output_length, &result, &masterResultsType);
  mechanic_check_mstat(mstat);

  mstat = buildMasterResultsType(handler->info->input_length, &inidata, &initialConditionsType);
  mechanic_check_mstat(mstat);

  /* Master can do something useful before computations. */
  query = mechanic_load_sym(handler, "in", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
  if (query) mstat = query(handler->mpi_size, handler->node, handler->info, handler->config, &inidata);
  mechanic_check_mstat(mstat);

  /* Align farm resolution for given method. */
  query = mechanic_load_sym(handler, "taskpool_resolution", MECHANIC_MODULE_ERROR, MECHANIC_NO_TEMPLATE);
  if (query) farm_res = query(handler->config->xres, handler->config->yres, handler->info, handler->config);
  if (farm_res > (handler->config->xres * handler->config->yres)) {
    mechanic_message(MECHANIC_MESSAGE_ERR,
      "Farm resolution should not exceed x*y!\n");
    mechanic_error(MECHANIC_ERR_SETUP);
  }

  /* FARM STARTS */

  /* Security check -- if farm resolution is greater than number of workers.
   * Needed when we have i.e. 3 workers and only one pixel to compute. */
  pixeldiff = farm_res - computed;
  if (pixeldiff >= handler->mpi_size) handler->nodes = handler->mpi_size;
  if (pixeldiff < handler->mpi_size) handler->nodes = pixeldiff+1;
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "mpi_size = %d, pixeldiff = %d, nodes = %d\n",
    handler->mpi_size, pixeldiff, handler->nodes);

  /* Send tasks to all workers,
   * remembering what the farm resolution is.*/

  /* No tasks left, terminate all workers. */
  if (pixeldiff == 0) {
    for (i = 1; i < handler->mpi_size; i++) {

      mechanic_message(MECHANIC_MESSAGE_WARN,
        "No tasks left, terminating worker %d.\n",i);

      /* Just dummy assignment */
      inidata.coords[0] = inidata.coords[1] = inidata.coords[2] = 0;

      MPI_Send(&inidata, 1, initialConditionsType, i,
        MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);

    } 

    goto finalize;
  }
    
  /* We don't want to have workers idle, so, if there are some idle workers
   * terminate them (when farm resolution < mpi size). */
 if (pixeldiff < handler->mpi_size) {
   for (i = handler->nodes; i < handler->mpi_size; i++) {
     mechanic_message(MECHANIC_MESSAGE_WARN,
       "Terminating not needed worker %d.\n", i);

     /* Just dummy assignment */
     inidata.coords[0] = inidata.coords[1] = inidata.coords[2] = 0;

     MPI_Send(&inidata, 1, initialConditionsType, i,
       MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);
    }
  } 
   
 for (i = 1; i < handler->nodes; i++) {

    handler->sendnode = i;
    npxc = map2d(npxc, handler, ptab, board);
    count++;

    inidata.coords[0] = ptab[0];
    inidata.coords[1] = ptab[1];
    inidata.coords[2] = ptab[2];

    mechanic_message(MECHANIC_MESSAGE_CONT,
        "Task [%04d, %04d, %04d] sended to node %04d\n",
        ptab[0], ptab[1], ptab[2], handler->sendnode);

    query = mechanic_load_sym(handler, "task_before_data_send", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
    if (query) mstat = query(handler->sendnode, handler->info, handler->config, &result);
    mechanic_check_mstat(mstat);

    MPI_Send(&inidata, 1, initialConditionsType, handler->sendnode,
        MECHANIC_MPI_DATA_TAG, MPI_COMM_WORLD);

    query = mechanic_load_sym(handler, "task_after_data_send", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
    if (query) mstat = query(handler->sendnode, handler->info, handler->config, &result);
    mechanic_check_mstat(mstat);
  }

  /* Receive data and send tasks. */
  
  while (1) {

    query = mechanic_load_sym(handler, "task_before_data_receive", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
    if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);

    MPI_Recv(&result, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG,
        MPI_COMM_WORLD, &mpi_status);

    handler->recvnode = mpi_status.MPI_SOURCE;

    totalnumofpx++;
    count--;

    query = mechanic_load_sym(handler, "task_after_data_receive", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
    if (query) mstat = query(handler->recvnode, handler->info, handler->config, &inidata, &result);
    mechanic_check_mstat(mstat);

    /* Copy data to checkpoint arrays */
    coordsarr[check][0] = result.coords[0];
    coordsarr[check][1] = result.coords[1];
    coordsarr[check][2] = result.coords[2];

    mechanic_message(MECHANIC_MESSAGE_CONT,
        "[%04d / %04d] Task [%04d, %04d, %04d] received from node %d\n",
        totalnumofpx, pixeldiff,  result.coords[0], result.coords[1],
        result.coords[2], handler->recvnode);

    for (j = 0; j < handler->info->output_length; j++) {
      resultarr[check][j] = result.data[j];
			mechanic_message(MECHANIC_MESSAGE_DEBUG, "%2.2f\t", result.data[j]);
    }

		mechanic_message(MECHANIC_MESSAGE_DEBUG, "\n");

    /* We write data only on checkpoint:
     * it is more healthier for disk, but remember --
     * if you set up checkpoints very often and your simulations are
     * very fast, your disks will not be happy
     */

    if ((((check+1) % handler->config->checkpoint) == 0) || (mechanic_ice(handler) == MECHANIC_ICE)) {

      /* Fortran interoperability:
       * Convert 2D coordinates array to 1D vector, as well as
       * results array */
      coordsvec = IntArrayToVec(coordsarr, handler->config->checkpoint, vecsize);
      resultsvec = DoubleArrayToVec(resultarr, handler->config->checkpoint, handler->info->output_length);

      query = mechanic_load_sym(handler, "task_before_checkpoint", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->nodes, handler->info, handler->config, coordsvec, resultsvec);
      mechanic_check_mstat(mstat);

      mstat = atCheckPoint(handler, check+1, coordsarr, board, resultarr);
      mechanic_check_mstat(mstat);
      check = 0;

      query = mechanic_load_sym(handler, "task_after_checkpoint", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->nodes, handler->info, handler->config, coordsvec, resultsvec);
      mechanic_check_mstat(mstat);

      FreeIntVec(coordsvec);
      FreeDoubleVec(resultsvec);

    } else {
      check++;
    }

    /* Abort if ICE signal has been received */
    if (mechanic_ice(handler) == MECHANIC_ICE) mechanic_abort(MECHANIC_ICE);

    /* Send next task to the worker */
    if (npxc < farm_res){

      handler->sendnode = handler->recvnode;

      npxc = map2d(npxc, handler, ptab, board);
      count++;

      inidata.coords[0] = ptab[0];
      inidata.coords[1] = ptab[1];
      inidata.coords[2] = ptab[2];

      query = mechanic_load_sym(handler, "task_before_data_send", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->sendnode, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      mechanic_message(MECHANIC_MESSAGE_CONT2,
        "Task [%04d, %04d, %04d] sended to node %d\n",
          inidata.coords[0], inidata.coords[1], inidata.coords[2], handler->sendnode);

      MPI_Send(&inidata, 1, initialConditionsType, handler->sendnode,
          MECHANIC_MPI_DATA_TAG, MPI_COMM_WORLD);

      query = mechanic_load_sym(handler, "task_after_data_send", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->sendnode, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

    } else {
      break;
    }
  } /* while(1) ends */

  /*
   * No more work to do, receive the outstanding results from the workers.
   * We've got exactly 'count' messages to receive.
   */
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Count is %d, Check is %d\n", count, check);

  for (i = 0; i < count; i++){

    query = mechanic_load_sym(handler, "task_before_data_receive", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
    if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
    mechanic_check_mstat(mstat);

    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Recv... ");

    MPI_Recv(&result, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG,
        MPI_COMM_WORLD, &mpi_status);

    handler->recvnode = mpi_status.MPI_SOURCE;

    mechanic_message(MECHANIC_MESSAGE_DEBUG, "done\n");
    totalnumofpx++;

    query = mechanic_load_sym(handler, "task_after_data_receive", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
    if (query) mstat = query(handler->recvnode, handler->info, handler->config, &inidata, &result);
    mechanic_check_mstat(mstat);

    /* Copy data to checkpoint arrays */
    coordsarr[check][0] = result.coords[0];
    coordsarr[check][1] = result.coords[1];
    coordsarr[check][2] = result.coords[2];

    for (j = 0; j < handler->info->output_length; j++) {
      resultarr[check][j] = result.data[j];
    }

    mechanic_message(MECHANIC_MESSAGE_CONT,
        "[%04d / %04d] Task [%04d, %04d, %04d] received from node %d\n",
        totalnumofpx, pixeldiff,  result.coords[0], result.coords[1],
        result.coords[2], handler->recvnode);
    /* There is a possibility that count > check, and then we have to perform
     * another checkpoint write */
    if ((((check+1) % handler->config->checkpoint) == 0) || (mechanic_ice(handler) == MECHANIC_ICE)) {

      /* Fortran interoperability:
       * Convert 2D coordinates array to 1D vector, as well as
       * results array */
      coordsvec = IntArrayToVec(coordsarr, handler->config->checkpoint, vecsize);
      resultsvec = DoubleArrayToVec(resultarr, handler->config->checkpoint, handler->info->output_length);

      query = mechanic_load_sym(handler, "task_before_checkpoint", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->nodes, handler->info, handler->config, coordsvec, resultsvec);
      mechanic_check_mstat(mstat);

      mstat = atCheckPoint(handler, check+1, coordsarr, board, resultarr);
      mechanic_check_mstat(mstat);
      check = 0;

      query = mechanic_load_sym(handler, "task_after_checkpoint", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->nodes, handler->info, handler->config, coordsvec, resultsvec);
      mechanic_check_mstat(mstat);

      FreeIntVec(coordsvec);
      FreeDoubleVec(resultsvec);

    } else {
      check++;
    }

    /* Abort if ICE signal has been received */
    if (mechanic_ice(handler) == MECHANIC_ICE) mechanic_abort(MECHANIC_ICE);

  }

  /* Write outstanding results to file
   * If farm_res is smaller than checkpoint interval, it means, we write
   * all data at once, and we have to handle before/task_after_checkpoint here
   * as well
   */

  /* Fortran interoperability:
   * Convert 2D coordinates array to 1D vector, as well as
   * results array */
  //coordsvec = IntArrayToVec(coordsarr, d->checkpoint, vecsize);
  //resultsvec = DoubleArrayToVec(resultarr, d->checkpoint, md->output_length);

  //query = mechanic_load_sym(handler, "node_task_before_checkpoint",
  //  "master_task_before_checkpoint", MECHANIC_MODULE_SILENT);
  //if (query) mstat = query(nodes, md, d, coordsvec, resultsvec);

  mstat = atCheckPoint(handler, check, coordsarr, board, resultarr);
  mechanic_check_mstat(mstat);

  //query = mechanic_load_sym(handler, "node_task_after_checkpoint",
  //    "master_task_after_checkpoint", MECHANIC_MODULE_SILENT);
  //if (query) mstat = query(nodes, md, d, coordsvec, resultsvec);

  //if(coordsvec) free(coordsvec);
  //if(resultsvec) free(resultsvec);

  /* Now, terminate the workers */
  for (i = 1; i < handler->nodes; i++) {

    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Terminating node %d\n", i); 
    inidata.coords[0] = inidata.coords[1] = inidata.coords[2] = 0;
    MPI_Send(&inidata, 1, initialConditionsType, i,
      MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);

  }

finalize:

  /* FARM ENDS */
  MPI_Type_free(&masterResultsType);
  MPI_Type_free(&initialConditionsType);

  /* Master can do something useful after the computations. */
  query = mechanic_load_sym(handler, "out", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
  if (query) mstat = query(handler->nodes, handler->node, handler->info, handler->config, &inidata, &result);
  mechanic_check_mstat(mstat);

  /* Release resources */
  FreeDoubleVec(result.data);
  FreeDoubleVec(inidata.data);
  FreeInt2D(coordsarr,handler->config->checkpoint);
  FreeDouble2D(resultarr,handler->config->checkpoint);
  FreeInt2D(board,handler->config->xres);

  return mstat;
}

