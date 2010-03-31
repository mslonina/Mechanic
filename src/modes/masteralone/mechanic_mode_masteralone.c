/*
 * MECHANIC Copyright (c) 2010, Mariusz Slonina (Nicolaus Copernicus University)
 * All rights reserved.
 * 
 * This file is part of MECHANIC code. 
 *
 * MECHANIC was created to help solving many numerical problems by providing tools
 * for improving scalability and functionality of the code. MECHANIC was released 
 * in belief it will be useful. If you are going to use this code, or its parts,
 * please consider referring to the authors either by the website or the user guide 
 * reference.
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
 *  - Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *  - Neither the name of the Nicolaus Copernicus University nor the names of 
 *    its contributors may be used to endorse or promote products derived from 
 *    this software without specific prior written permission.
 *  
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, 
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 */

#include "mechanic.h"
#include "mechanic_internals.h"
#include "mechanic_mode_masteralone.h"

int mechanic_mode_masteralone(int node, void* handler, moduleInfo* md, configData* d){

  int i = 0, j = 0, farm_res = 0, mstat = 0, tab[3], check = 0;
  masterData rawdata;
   
  /* Checkpoint storage */
  int** coordsarr;
  MECHANIC_DATATYPE **resultarr;
   
  /* Restart mode board */
  int** board;
   
  module_query_void_f qpc, qpx, qpb;
  module_query_int_f qr;
 
  /* Allocate memory */
  rawdata.res = realloc(NULL, ((uintptr_t)md->mrl)*sizeof(MECHANIC_DATATYPE));
  if(rawdata.res == NULL) mechanic_error(MECHANIC_ERR_MEM);
   
  coordsarr = malloc(sizeof(uintptr_t)*((uintptr_t)d->checkpoint + 1));
  if(coordsarr == NULL) mechanic_error(MECHANIC_ERR_MEM);

  resultarr = malloc(sizeof(MECHANIC_DATATYPE)*((uintptr_t)d->checkpoint + 1));
  if(resultarr == NULL) mechanic_error(MECHANIC_ERR_MEM);

  for(i = 0; i < d->checkpoint; i++){
    coordsarr[i] = malloc(sizeof(uintptr_t)*3);
    if(coordsarr[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);
     
    resultarr[i] = malloc(sizeof(MECHANIC_DATATYPE)*((uintptr_t)md->mrl));
    if(resultarr[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);
  }

  /* Allocate memory for board */
  if(d->restartmode == 1){
    board = malloc(sizeof(uintptr_t)*((uintptr_t)d->xres));
    if(board == NULL) mechanic_error(MECHANIC_ERR_MEM);

    for(i = 0; i < d->xres; i++) 
      board[i] = malloc(sizeof(uintptr_t)*((uintptr_t)d->yres));
      if(board[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);
  }

  if(d->restartmode == 1) H5readBoard(d, board);

  /* Master can do something useful before computations,
   * even in masteralone mode */
  query = load_sym(handler, md, "node_in", "master_in", MECHANIC_MODULE_SILENT);
  if(query) mstat = query(mpi_size, node, md, d);

  /* Align farm resolution for given method. */
  if(d->method == 0) farm_res = d->xres*d->yres; /* one pixel per each slave */
  if(d->method == 1) farm_res = d->xres; /* sliceX */
  if(d->method == 2) farm_res = d->yres; /* sliceY */
  if(d->method == 6){
    qr = load_sym(handler, md, "farmResolution", "farmResolution", MECHANIC_MODULE_ERROR);
    if(qr) farm_res = qr(d->xres, d->yres, md);
  }
 
  /* Perform farm operations */
  for(i = 0; i < farm_res; i++){

    map2d(i, handler, md, d, tab);

    if(d->method == 0){
      rawdata.coords[0] = tab[0];
      rawdata.coords[1] = tab[1];
      rawdata.coords[2] = tab[2];
    }

    if(d->method == 6){
      qpc = load_sym(handler, md, "pixelCoords", "pixelCoords", MECHANIC_MODULE_ERROR);
      if(qpc) mstat = qpc(node, tab, md, d, &rawdata);
    } 
    
    qpb = load_sym(handler, md, "node_beforePixelCompute", "master_beforePixelCompute", MECHANIC_MODULE_SILENT);
    if(qpb) mstat = qpb(node, md, d, &rawdata);

    /* PIXEL COMPUTATION */
    qpx = load_sym(handler, md, "pixelCompute", "pixelCompute", MECHANIC_MODULE_ERROR);
    if(qpx) mstat = qpx(node, md, d, &rawdata);
          
    qpb = load_sym(handler, md, "node_afterPixelCompute", "master_afterPixelCompute", MECHANIC_MODULE_SILENT);
    if(qpb) mstat = qpb(node, md, d, &rawdata);
 
    /* Copy data to checkpoint arrays */
    coordsarr[check][0] = rawdata.coords[0];
    coordsarr[check][1] = rawdata.coords[1];
    coordsarr[check][2] = rawdata.coords[2];

		mechanic_message(MECHANIC_MESSAGE_CONT, "MASTER [%d, %d, %d]\t", rawdata.coords[0], rawdata.coords[1], rawdata.coords[2]);
     
    for(j = 0; j < md->mrl; j++){
      resultarr[check][j] = rawdata.res[j];
      mechanic_message(MECHANIC_MESSAGE_DEBUG,"%2.2f\t",rawdata.res[j]);
    }
		mechanic_message(MECHANIC_MESSAGE_DEBUG,"\n");

    check++;

    if(check % d->checkpoint == 0){/* FIX ME: add UPS checks */ 
      mstat = atCheckPoint(check, coordsarr, board, resultarr, md, d);
      check = 0;
    }

  }

  /* Write outstanding results */
  if(check > 0)  mstat = atCheckPoint(check, coordsarr, board, resultarr, md, d);

  /* FARM ENDS */
    
  /* Master can do something useful after the computations. */
  query = load_sym(handler, md, "node_out", "master_out", MECHANIC_MODULE_SILENT);
  if(query) mstat = query(1, node, md, d, &rawdata);

  free(rawdata.res);
 
  for(i = 0; i < d->checkpoint; i++){
    free(coordsarr[i]);
    free(resultarr[i]);
  }

  free(coordsarr);
  free(resultarr);

  if(d->restartmode == 1){
    for(i = 0; i < d->xres; i++) free(board[i]);
    free(board);
  }
 
  return 0;
}
