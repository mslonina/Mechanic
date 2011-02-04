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
#include "mechanic_mode_masteralone.h"

int mechanic_mode_masteralone(int mpi_size, int node, mechanic_internals handler,
    moduleInfo* md, configData* d){

  int j = 0, farm_res = 0, mstat = 0, tab[3], check = 0;
  int npxc = 0;

  masterData result;
  masterData inidata;

  /* Checkpoint storage */
  int** coordsarr;
  MECHANIC_DATATYPE **resultarr;

  /* Restart mode board */
  int** board;
  int computed = 0;
  int pixeldiff = 0;
  int totalnumofpx = 0;

  module_query_int_f query;

  /* Allocate memory */
  result.res = AllocateDoubleVec(md->mrl);
  inidata.res = AllocateDoubleVec(md->irl);

  coordsarr = AllocateInt2D(d->checkpoint,3);
  resultarr = AllocateDouble2D(d->checkpoint,md->mrl);

  /* Allocate memory for board */
  board = AllocateInt2D(d->xres,d->yres);

  computed = H5readBoard(d, board);

  /* Master can do something useful before computations,
   * even in masteralone mode */
  query = mechanic_load_sym(handler, "in", MECHANIC_MODULE_SILENT);
  if (query) mstat = query(mpi_size, node, md, d, &inidata);

  /* Align farm resolution for given method. */
  query = mechanic_load_sym(handler, "farmResolution", MECHANIC_MODULE_ERROR);
  if (query) farm_res = query(d->xres, d->yres, md, d);

  pixeldiff = farm_res - computed;

  /* Perform farm operations */
  while (1) {

    npxc = map2d(npxc, handler, md, d, tab, board);
    totalnumofpx++;

    inidata.coords[0] = tab[0];
    inidata.coords[1] = tab[1];
    inidata.coords[2] = tab[2];

    query = mechanic_load_sym(handler, "pixelCoords", MECHANIC_MODULE_ERROR);
    if (query) mstat = query(node, tab, md, d, &inidata, &result);

    query = mechanic_load_sym(handler, "preparePixel", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(node, md, d, &inidata, &result);

    query = mechanic_load_sym(handler, "beforeProcessPixel", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(node, md, d, &inidata, &result);

    /* PIXEL COMPUTATION */
    query = mechanic_load_sym(handler, "processPixel", MECHANIC_MODULE_ERROR);
    if (query) mstat = query(node, md, d, &inidata, &result);

    query = mechanic_load_sym(handler, "afterProcessPixel", MECHANIC_MODULE_SILENT);
    if (query) mstat = query(node, md, d, &inidata, &result);

    /* Copy data to checkpoint arrays */
    coordsarr[check][0] = result.coords[0];
    coordsarr[check][1] = result.coords[1];
    coordsarr[check][2] = result.coords[2];

		mechanic_message(MECHANIC_MESSAGE_DEBUG, "MASTER [%d, %d, %d]\t",
        result.coords[0], result.coords[1], result.coords[2]);

    for (j = 0; j < md->mrl; j++) {
      resultarr[check][j] = result.res[j];
      mechanic_message(MECHANIC_MESSAGE_DEBUG, "%2.2f\t", result.res[j]);
    }

		mechanic_message(MECHANIC_MESSAGE_DEBUG, "\n");

    check++;

    if (check % d->checkpoint == 0 || mechanic_ups() < 0) {
      query = mechanic_load_sym(handler, "beforeCheckpoint", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(mpi_size, md, d, coordsarr, resultarr);

      mstat = atCheckPoint(check, coordsarr, board, resultarr, md, d);
      check = 0;

      query = mechanic_load_sym(handler, "afterCheckpoint", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(mpi_size, md, d, coordsarr, resultarr);
    }

    mechanic_message(MECHANIC_MESSAGE_CONT,
        "[%04d / %04d] Pixel [%04d, %04d, %04d] computed\n",
        totalnumofpx, pixeldiff,  result.coords[0], result.coords[1],
        result.coords[2]);

    if (npxc >= farm_res) break;
  }

  /* Write outstanding results */
  if (check > 0) {
      query = mechanic_load_sym(handler, "beforeCheckpoint", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(mpi_size, md, d, coordsarr, resultarr);
      mstat = atCheckPoint(check, coordsarr, board, resultarr, md, d);
      query = mechanic_load_sym(handler, "afterCheckpoint", MECHANIC_MODULE_SILENT);
      if (query) mstat = query(mpi_size, md, d, coordsarr, resultarr);
  }

  /* FARM ENDS */

  /* Master can do something useful after the computations. */
  query = mechanic_load_sym(handler, "out", MECHANIC_MODULE_SILENT);
  if (query) mstat = query(1, node, md, d, &inidata, &result);

  FreeDoubleVec(result.res);
  FreeDoubleVec(inidata.res);
  FreeInt2D(coordsarr,d->checkpoint);
  FreeDouble2D(resultarr,d->checkpoint);
  FreeInt2D(board,d->xres);

  return mstat;
}

