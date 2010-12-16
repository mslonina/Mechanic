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

/* SLAVE */
int mechanic_mode_farm_slave(int mpi_size, int node, module_handler handler,
    moduleInfo* md, configData* d){

  int tab[3];
  int mstat;

  module_query_void_f qbeforeS, qafterS, qbeforeR, qafterR, qpx, qpc, qpb;

  masterData result;
  masterData inidata;

  MPI_Datatype masterResultsType;
  MPI_Datatype initialConditionsType;
  MPI_Status mpi_status;

  /* Allocate memory */
  result.res = AllocateDoubleVec(md->mrl);
  inidata.res = AllocateDoubleVec(md->irl);

  /* Build derived type for master result and initial condition */
  mstat = buildMasterResultsType(md->mrl, &result, &masterResultsType);
  mechanic_check_mstat(mstat);
  mstat = buildMasterResultsType(md->irl, &inidata, &initialConditionsType);
  mechanic_check_mstat(mstat);

  /* Slave can do something useful before __all__ computations */
  query = mechanic_load_sym(handler, d->module, "node_in", "slave_in",
      MECHANIC_MODULE_SILENT);
  if (query) mstat = query(mpi_size, node, md, d, &inidata);
  mechanic_check_mstat(mstat);

  qbeforeR = mechanic_load_sym(handler, d->module, "node_beforeReceive",
      "slave_beforeReceive", MECHANIC_MODULE_SILENT);
  if (qbeforeR) mstat = qbeforeR(node, md, d, &inidata, &result);
  mechanic_check_mstat(mstat);

  MPI_Recv(&inidata, 1, initialConditionsType, MECHANIC_MPI_DEST, MPI_ANY_TAG,
      MPI_COMM_WORLD, &mpi_status);

  qafterR = mechanic_load_sym(handler, d->module, "node_afterReceive",
      "slave_afterReceive", MECHANIC_MODULE_SILENT);
  if (qafterR) mstat = qafterR(node, md, d, &inidata, &result);
  mechanic_check_mstat(mstat);

  while (1) {

    if (mpi_status.MPI_TAG == MECHANIC_MPI_TERMINATE_TAG) break;
    if (mpi_status.MPI_TAG == MECHANIC_MPI_DATA_TAG) {

      tab[0] = inidata.coords[0];
      tab[1] = inidata.coords[1];
      tab[2] = inidata.coords[2];

      qpc = mechanic_load_sym(handler, d->module, "pixelCoords", "pixelCoords",
        MECHANIC_MODULE_ERROR);
      if (qpc) mstat = qpc(node, tab, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

      mechanic_message(MECHANIC_MESSAGE_DEBUG, "SLAVE[%d]: PTAB[%d, %d, %d]\n",
          node, inidata.coords[0], inidata.coords[1], inidata.coords[2]);
      mechanic_message(MECHANIC_MESSAGE_DEBUG, "SLAVE[%d]: RTAB[%d, %d, %d]\n",
          node, result.coords[0], result.coords[1], result.coords[2]);

      qpb = mechanic_load_sym(handler, d->module, "node_preparePixel",
          "slave_preparePixel", MECHANIC_MODULE_SILENT);
      if (qpb) mstat = qpb(node, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

      qpb = mechanic_load_sym(handler, d->module, "node_beforeProcessPixel",
          "slave_beforeProcessPixel", MECHANIC_MODULE_SILENT);
      if (qpb) mstat = qpb(node, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

      /* PIXEL COMPUTATION */
      qpx = mechanic_load_sym(handler, d->module, "processPixel", "processPixel",
          MECHANIC_MODULE_ERROR);
      if(qpx) mstat = qpx(node, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

      qpb = mechanic_load_sym(handler, d->module, "node_afterProcessPixel",
          "slave_afterProcessPixel", MECHANIC_MODULE_SILENT);
      if (qpb) mstat = qpb(node, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

      qbeforeS = mechanic_load_sym(handler, d->module, "node_beforeSend",
          "slave_beforeSend", MECHANIC_MODULE_SILENT);
      if (qbeforeS) mstat = qbeforeS(node, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

#ifdef IPM
      MPI_Pcontrol(1,"slave_send_result");
#endif

      MPI_Send(&result, 1, masterResultsType, MECHANIC_MPI_DEST,
          MECHANIC_MPI_RESULT_TAG, MPI_COMM_WORLD);

#ifdef IPM
      MPI_Pcontrol(-1,"slave_send_result");
#endif

      qafterS = mechanic_load_sym(handler, d->module, "node_afterSend",
          "slave_afterSend", MECHANIC_MODULE_SILENT);
      if (qafterS) mstat = qafterS(node, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

      qbeforeR = mechanic_load_sym(handler, d->module, "node_beforeReceive",
          "slave_beforeReceive", MECHANIC_MODULE_SILENT);
      if (qbeforeR) mstat = qbeforeR(node, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

#ifdef IPM
      MPI_Pcontrol(1,"slave_recv_pixel");
#endif

      MPI_Recv(&inidata, 1, initialConditionsType, MECHANIC_MPI_DEST,
          MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);

#ifdef IPM
      MPI_Pcontrol(-1,"slave_recv_pixel");
#endif

      qafterR = mechanic_load_sym(handler, d->module, "node_afterReceive",
          "slave_afterReceive", MECHANIC_MODULE_SILENT);
      if (qafterR) mstat = qafterR(node, md, d, &inidata, &result);
      mechanic_check_mstat(mstat);

    }
  } /* while (1) */

    /* Slave can do something useful after computations. */
    query = mechanic_load_sym(handler, d->module, "node_out", "slave_out",
        MECHANIC_MODULE_SILENT);
    if (query) mstat = query(mpi_size, node, md, d, &inidata, &result);
    mechanic_check_mstat(mstat);

    MPI_Type_free(&masterResultsType);
    MPI_Type_free(&initialConditionsType);

    FreeDoubleVec(result.res);
    FreeDoubleVec(inidata.res);

    return mstat;
}

