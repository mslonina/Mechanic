/**
 * @file
 * The Task Farm Mode -- The Worker
 */
#include "mechanic.h"
#include "mechanic_internals.h"
#include "mechanic_mode_farm.h"

int mechanic_mode_farm_worker(mechanic_internals *handler) {

  int tab[3];
  int mstat;

  module_query_int_f query;

  TaskData result;
  TaskData inidata;

  MPI_Datatype masterResultsType;
  MPI_Datatype initialConditionsType;
  MPI_Status mpi_status;

  /* Allocate memory */
  result.data = AllocateDoubleVec(handler->info->output_length);
  inidata.data = AllocateDoubleVec(handler->info->input_length);

  /* Build derived type for master result and initial condition */
  mstat = buildMasterResultsType(handler->info->output_length, &result, &masterResultsType);
  mechanic_check_mstat(mstat);
  mstat = buildMasterResultsType(handler->info->input_length, &inidata, &initialConditionsType);
  mechanic_check_mstat(mstat);

  /* Worker can do something useful before __all__ computations */
  query = mechanic_load_sym(handler, "in", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
  if (query) mstat = query(handler->mpi_size, handler->node, handler->info, handler->config, &inidata);
  mechanic_check_mstat(mstat);

  query = mechanic_load_sym(handler, "task_before_data_receive", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
  if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
  mechanic_check_mstat(mstat);

  MPI_Recv(&inidata, 1, initialConditionsType, MECHANIC_MPI_DEST, MPI_ANY_TAG,
      MPI_COMM_WORLD, &mpi_status);

  if (mpi_status.MPI_TAG == MECHANIC_MPI_TERMINATE_TAG) {
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Node %d recevied tag %d\n", handler->node, mpi_status.MPI_TAG);
    goto finalize;
  }

  query = mechanic_load_sym(handler, "task_after_data_receive", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
  if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
  mechanic_check_mstat(mstat);

  while (1) {

    if (mpi_status.MPI_TAG == MECHANIC_MPI_TERMINATE_TAG) break;
    if (mpi_status.MPI_TAG == MECHANIC_MPI_DATA_TAG) {

      tab[0] = inidata.coords[0];
      tab[1] = inidata.coords[1];
      tab[2] = inidata.coords[2];

      query = mechanic_load_sym(handler, "task_coordinates_assign", MECHANIC_MODULE_ERROR, MECHANIC_NO_TEMPLATE);
      if (query) mstat = query(handler->node, tab, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      mechanic_message(MECHANIC_MESSAGE_DEBUG, "WORKER[%d]: PTAB[%d, %d, %d]\n",
          handler->node, inidata.coords[0], inidata.coords[1], inidata.coords[2]);
      mechanic_message(MECHANIC_MESSAGE_DEBUG, "WORKER[%d]: RTAB[%d, %d, %d]\n",
          handler->node, result.coords[0], result.coords[1], result.coords[2]);

      query = mechanic_load_sym(handler, "task_prepare", MECHANIC_MODULE_SILENT, MECHANIC_NO_TEMPLATE);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      query = mechanic_load_sym(handler, "task_before_process", MECHANIC_MODULE_SILENT, MECHANIC_NO_TEMPLATE);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      /* PIXEL COMPUTATION */
      query = mechanic_load_sym(handler, "task_process", MECHANIC_MODULE_ERROR, MECHANIC_NO_TEMPLATE);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      query = mechanic_load_sym(handler, "task_after_process", MECHANIC_MODULE_SILENT, MECHANIC_NO_TEMPLATE);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      query = mechanic_load_sym(handler, "task_before_data_send", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
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

      query = mechanic_load_sym(handler, "task_after_data_send", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      query = mechanic_load_sym(handler, "task_before_data_receive", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
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

      query = mechanic_load_sym(handler, "task_after_data_receive", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->node, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

    }
  } /* while (1) */

finalize:

    /* Worker can do something useful after computations. */
    query = mechanic_load_sym(handler, "out", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
    if (query) mstat = query(handler->mpi_size, handler->node, handler->info, handler->config, &inidata, &result);
    mechanic_check_mstat(mstat);

    MPI_Type_free(&masterResultsType);
    MPI_Type_free(&initialConditionsType);

    FreeDoubleVec(result.data);
    FreeDoubleVec(inidata.data);

    mechanic_message(MECHANIC_MESSAGE_DEBUG, "Node %d terminated\n", handler->node);
    return mstat;
}

