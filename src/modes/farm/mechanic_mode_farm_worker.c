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

/* SLAVE */
int mechanic_mode_farm_worker(mechanic_internals *handler) {

  int tab[3];
  int mstat;

  module_query_int_f query;

  masterData result;
  masterData inidata;

  MPI_Datatype masterResultsType;
  MPI_Datatype initialConditionsType;
  MPI_Status mpi_status;

  /* Allocate memory */
  result.res = AllocateDoubleVec(handler->info->mrl);
  inidata.res = AllocateDoubleVec(handler->info->irl);

  /* Build derived type for master result and initial condition */
  mstat = buildMasterResultsType(handler->info->mrl, &result, &masterResultsType);
  mechanic_check_mstat(mstat);
  mstat = buildMasterResultsType(handler->info->irl, &inidata, &initialConditionsType);
  mechanic_check_mstat(mstat);

  /* Slave can do something useful before __all__ computations */
  query = mechanic_load_sym(handler, "in", MECHANIC_MODULE_SILENT);
  if (query) mstat = query(handler->mpi_size, handler->node, handler->info, handler->config, &inidata);
  mechanic_check_mstat(mstat);

  query = mechanic_load_sym(handler, "task_before_data_receive", MECHANIC_MODULE_SILENT);
  if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
  mechanic_check_mstat(mstat);

  MPI_Recv(&inidata, 1, initialConditionsType, MECHANIC_MPI_DEST, MPI_ANY_TAG,
      MPI_COMM_WORLD, &mpi_status);

  if (mpi_status.MPI_TAG == MECHANIC_MPI_TERMINATE_TAG) {
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Node %d recevied tag %d\n", handler->node, mpi_status.MPI_TAG);
    goto finalize;
  }

  query = mechanic_load_sym(handler, "task_after_data_receive", MECHANIC_MODULE_SILENT);
  if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
  mechanic_check_mstat(mstat);

  while (1) {

    if (mpi_status.MPI_TAG == MECHANIC_MPI_TERMINATE_TAG) break;
    if (mpi_status.MPI_TAG == MECHANIC_MPI_DATA_TAG) {

      tab[0] = inidata.coords[0];
      tab[1] = inidata.coords[1];
      tab[2] = inidata.coords[2];

      query = mechanic_load_sym(handler, "task_coordinates_assign", MECHANIC_MODULE_ERROR);
      if (query) mstat = query(handler->node, tab, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      mechanic_message(MECHANIC_MESSAGE_DEBUG, "SLAVE[%d]: PTAB[%d, %d, %d]\n",
          handler->node, inidata.coords[0], inidata.coords[1], inidata.coords[2]);
      mechanic_message(MECHANIC_MESSAGE_DEBUG, "SLAVE[%d]: RTAB[%d, %d, %d]\n",
          handler->node, result.coords[0], result.coords[1], result.coords[2]);

      query = mechanic_load_sym(handler, "task_prepare", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      query = mechanic_load_sym(handler, "task_before_process", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      /* PIXEL COMPUTATION */
      query = mechanic_load_sym(handler, "task_process", MECHANIC_MODULE_ERROR);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      query = mechanic_load_sym(handler, "task_after_process", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      query = mechanic_load_sym(handler, "task_before_data_send", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

#ifdef IPM
      MPI_Pcontrol(1,"worker_send_result");
#endif

      MPI_Send(&result, 1, masterResultsType, MECHANIC_MPI_DEST,
          MECHANIC_MPI_RESULT_TAG, MPI_COMM_WORLD);

#ifdef IPM
      MPI_Pcontrol(-1,"worker_send_result");
#endif

      query = mechanic_load_sym(handler, "task_after_data_send", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      query = mechanic_load_sym(handler, "task_before_data_receive", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

#ifdef IPM
      MPI_Pcontrol(1,"worker_recv_pixel");
#endif

      MPI_Recv(&inidata, 1, initialConditionsType, MECHANIC_MPI_DEST,
          MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);

#ifdef IPM
      MPI_Pcontrol(-1,"worker_recv_pixel");
#endif

      query = mechanic_load_sym(handler, "task_after_data_receive", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

    }
  } /* while (1) */

finalize:

    /* Slave can do something useful after computations. */
    query = mechanic_load_sym(handler, "out", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(handler->mpi_size, handler->node, handler->info, handler->config, &inidata, &result);
    mechanic_check_mstat(mstat);

    MPI_Type_free(&masterResultsType);
    MPI_Type_free(&initialConditionsType);

    FreeDoubleVec(result.res);
    FreeDoubleVec(inidata.res);

    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Node %d terminated\n", handler->node);
    return mstat;
}

