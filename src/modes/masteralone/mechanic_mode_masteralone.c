/**
 * @file
 * The Masteralone mode
 */
#include "mechanic.h"
#include "mechanic_internals.h"
#include "mechanic_mode_masteralone.h"

int mechanic_mode_masteralone(mechanic_internals* handler) {

  int j = 0, farm_res = 0, mstat = 0, tab[3], check = 0;
  int npxc = 0;

  TaskData result;
  TaskData inidata;

  /* Checkpoint storage */
  int vecsize;
  int* coordsvec;
  int** coordsarr;
  MECHANIC_DATATYPE *resultsvec;
  MECHANIC_DATATYPE **resultarr;

  /* Restart mode board */
  int** board;
  int computed = 0;
  int pixeldiff = 0;
  int totalnumofpx = 0;

  module_query_int_f query;

  /* Allocate memory */
  result.data = AllocateDoubleVec(handler->info->output_length);
  inidata.data = AllocateDoubleVec(handler->info->input_length);

  coordsarr = AllocateInt2D(handler->config->checkpoint,3);
  resultarr = AllocateDouble2D(handler->config->checkpoint,handler->info->output_length);

  /* Allocate memory for board */
  board = AllocateInt2D(handler->config->xres,handler->config->yres);

  vecsize = 3;

  /* For the sake of simplicity we read board everytime,
   * both in restart and clean simulation mode */

  mstat = H5readBoard(handler->config, board, &computed);
  mechanic_check_mstat(mstat);
  mechanic_message(MECHANIC_MESSAGE_DEBUG, "Num of computed pixels = %d\n", computed);

  /* Master can do something useful before computations,
   * even in masteralone mode */
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

  pixeldiff = farm_res - computed;

  if (pixeldiff == 0) goto finalize;

  /* Perform farm operations */
  while (1) {

    npxc = map2d(npxc, handler, tab, board);
    totalnumofpx++;

    inidata.coords[0] = tab[0];
    inidata.coords[1] = tab[1];
    inidata.coords[2] = tab[2];

    result.coords[0] = tab[0];
    result.coords[1] = tab[1];
    result.coords[2] = tab[2];
    
    mechanic_message(MECHANIC_MESSAGE_DEBUG, "TAB [%d, %d, %d]\t",
      tab[0], tab[1], tab[2]);

    query = mechanic_load_sym(handler, "task_coordinates_assign", MECHANIC_MODULE_ERROR, MECHANIC_NO_TEMPLATE);
    if (query) mstat = query(handler->node, tab, handler->info, handler->config, &inidata, &result);
    mechanic_check_mstat(mstat);

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

    /* Copy data to checkpoint arrays */
    coordsarr[check][0] = result.coords[0];
    coordsarr[check][1] = result.coords[1];
    coordsarr[check][2] = result.coords[2];

		mechanic_message(MECHANIC_MESSAGE_DEBUG, "MASTER [%d, %d, %d]\t",
        result.coords[0], result.coords[1], result.coords[2]);

    for (j = 0; j < handler->info->output_length; j++) {
      resultarr[check][j] = result.data[j];
      mechanic_message(MECHANIC_MESSAGE_DEBUG, "%2.2f\t", result.data[j]);
    }

		mechanic_message(MECHANIC_MESSAGE_DEBUG, "\n");

    if (((check+1) % handler->config->checkpoint) == 0 || (mechanic_ice(handler) == MECHANIC_ICE)) {
      
      /* Fortran interoperability:
       * Convert 2D coordinates array to 1D vector, as well as
       * results array */
      coordsvec = IntArrayToVec(coordsarr, handler->config->checkpoint, vecsize);
      resultsvec = DoubleArrayToVec(resultarr, handler->config->checkpoint, handler->info->output_length);
      
      query = mechanic_load_sym(handler, "task_before_checkpoint", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->mpi_size, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      mstat = atCheckPoint(handler, check+1, coordsarr, board, resultarr);
      mechanic_check_mstat(mstat);
      check = 0;

      query = mechanic_load_sym(handler, "task_after_checkpoint", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->mpi_size, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      FreeIntVec(coordsvec);
      FreeDoubleVec(resultsvec);
    }

    check++;
    
    /* Abort if ICE signal has been received */
    if (mechanic_ice(handler) == MECHANIC_ICE) mechanic_abort(MECHANIC_ICE);

    mechanic_message(MECHANIC_MESSAGE_CONT,
        "[%04d / %04d] Task [%04d, %04d, %04d] computed\n",
        totalnumofpx, pixeldiff,  result.coords[0], result.coords[1],
        result.coords[2]);

    if (npxc >= farm_res) break;
  }

  /* Write outstanding results */
  if (check > 0) {
      
      /* Fortran interoperability:
       * Convert 2D coordinates array to 1D vector, as well as
       * results array */
      coordsvec = IntArrayToVec(coordsarr, handler->config->checkpoint, vecsize);
      resultsvec = DoubleArrayToVec(resultarr, handler->config->checkpoint, handler->info->output_length);
    
      query = mechanic_load_sym(handler, "task_before_checkpoint", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->mpi_size, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);

      mstat = atCheckPoint(handler, check+1, coordsarr, board, resultarr);
      mechanic_check_mstat(mstat);

      query = mechanic_load_sym(handler, "task_after_checkpoint", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
      if (query) mstat = query(handler->mpi_size, handler->info, handler->config, &inidata, &result);
      mechanic_check_mstat(mstat);
      
      FreeIntVec(coordsvec);
      FreeDoubleVec(resultsvec);
  }
    
  /* Abort if ICE signal has been received */
  if (mechanic_ice(handler) == MECHANIC_ICE) mechanic_abort(MECHANIC_ICE);

finalize:

  /* Master can do something useful after the computations. */
  query = mechanic_load_sym(handler, "out", MECHANIC_MODULE_SILENT, MECHANIC_TEMPLATE);
  if (query) mstat = query(handler->mpi_size, handler->info, handler->config, &inidata, &result);
  mechanic_check_mstat(mstat);

  FreeDoubleVec(result.data);
  FreeDoubleVec(inidata.data);
  FreeInt2D(coordsarr,handler->config->checkpoint);
  FreeDouble2D(resultarr,handler->config->checkpoint);
  FreeInt2D(board,handler->config->xres);

  return mstat;
}

