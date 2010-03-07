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
#include "mechanic_mode_farm.h"

/*
 * MASTER
 *
 */
int mechanic_mode_farm_master(int node, void* handler, moduleInfo* md, configData* d){

   int* tab;
   int i = 0, k = 0, j = 0, nodes = 0, farm_res = 0;
   int npxc = 0; //number of all sent pixels
   int count = 0; //number of pixels to receive
   int check = 0;

   int mstat;
   
   masterData *rawdata;

   // Checkpoint storage 
   int** coordsarr;
   MECHANIC_DATATYPE **resultarr;
   
   // Restart mode board
   int** board;

   MPI_Status mpi_status;
   MPI_Datatype masterResultsType;
   
   module_query_void_f queryIN, queryOUT, qbeforeS, qafterS, qbeforeR, qafterR;
   module_query_int_f qr;
 
   // Allocate memory for rawdata.res array.
   rawdata = malloc(sizeof(masterData) + (md->mrl-1)*sizeof(MECHANIC_DATATYPE));
   if(rawdata == NULL) mechanic_error(MECHANIC_ERR_MEM);

   clearArray(rawdata->res, ITEMS_IN_ARRAY(rawdata->res));
   
  coordsarr = malloc(sizeof(int*)*d->checkpoint);
  if(coordsarr == NULL) mechanic_error(MECHANIC_ERR_MEM);

  resultarr = malloc(sizeof(MECHANIC_DATATYPE*)*d->checkpoint);
  if(resultarr == NULL) mechanic_error(MECHANIC_ERR_MEM);

  for(i = 0; i < d->checkpoint; i++){
     coordsarr[i] = malloc(sizeof(int*)*3);
     if(coordsarr[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);
     
     resultarr[i] = malloc(sizeof(MECHANIC_DATATYPE*)*md->mrl);
     if(resultarr[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);
     
     clearArray(resultarr[i],ITEMS_IN_ARRAY(resultarr[i]));
  }

   // Allocate memory for board
   if(d->restartmode == 1){
    board = malloc(sizeof(int*)*d->xres);
    if(board == NULL) mechanic_error(MECHANIC_ERR_MEM);

    for(i = 0; i < d->xres; i++) 
      board[i] = malloc(sizeof(int*)*d->yres);
      if(board[i] == NULL) mechanic_error(MECHANIC_ERR_MEM);
    
   }

   if(d->restartmode == 1) H5readBoard(d, board);

   // Build derived type for master result
   mstat = buildMasterResultsType(md->mrl, rawdata, &masterResultsType);

   // Master can do something useful before computations.
   query = load_sym(handler, md, "masterIN", MECHANIC_MODULE_SILENT);
   if(query) mstat = query(mpi_size, md, d);

   // Align farm resolution for given method.
   if (d->method == 0) farm_res = d->xres*d->yres; //one pixel per each slave
   if (d->method == 1) farm_res = d->xres; //sliceX
   if (d->method == 2) farm_res = d->yres; //sliceY
   if (d->method == 6){
     qr = load_sym(handler, md, "farmResolution", MECHANIC_MODULE_ERROR);
     if(qr) farm_res = qr(d->xres, d->yres, md);
   }
 
   // FARM STARTS

   // Security check -- if farm resolution is greater than number of slaves.
   // Needed when we have i.e. 3 slaves and only one pixel to compute.
   if (farm_res > mpi_size) nodes = mpi_size;
   if (farm_res == mpi_size) nodes = mpi_size;
   if (farm_res < mpi_size) nodes = farm_res + 1;

   // Send tasks to all slaves,
   // remembering what the farm resolution is.
   for (i = 1; i < nodes; i++){
     qbeforeS = load_sym(handler, md,"master_beforeSend", MECHANIC_MODULE_SILENT);
     if(qbeforeS) mstat = qbeforeS(i, md, d, rawdata);

     MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, i, MECHANIC_MPI_DATA_TAG, MPI_COMM_WORLD);
     
     qafterS = load_sym(handler, md, "master_afterSend", MECHANIC_MODULE_SILENT);
     if(qafterS) mstat = qafterS(i, md, d, rawdata);
     
     count++;
     npxc++;
   }
   
   // We don't want to have slaves idle, so, if there are some idle slaves
   // terminate them (when farm resolution < mpi size).
   if(farm_res < mpi_size){
    for(i = nodes; i < mpi_size; i++){
      printf("-> Terminating idle slave %d.\n", i);
      MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, i, MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);
    }
   }

   // Receive data and send tasks.
   while (1) {
    
     qbeforeR = load_sym(handler, md, "master_beforeReceive", MECHANIC_MODULE_SILENT);
     if(qbeforeR) mstat = qbeforeR(md, d, rawdata);
      
     MPI_Recv(rawdata, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
     count--;
      
      qafterR = load_sym(handler, md, "master_afterReceive", MECHANIC_MODULE_SILENT);
      if(qafterR) mstat = qafterR(mpi_status.MPI_SOURCE, md, d, rawdata);
   
      // Copy data to checkpoint arrays
      coordsarr[check][0] = rawdata->coords[0];
      coordsarr[check][1] = rawdata->coords[1];
      coordsarr[check][2] = rawdata->coords[2];
     
      for(j = 0; j < md->mrl; j++){
        resultarr[check][j] = rawdata->res[j];
      }
      check++;

      /* We write data only on checkpoint:
       * it is more healthier for disk, but remember --
       * if you set up checkpoints very often and your simulations are
       * very fast, your disks will not be happy
       * */

      if(check % d->checkpoint == 0){//FIX ME: add UPS checks  
        mstat = atCheckPoint(check, coordsarr, board, resultarr, md, d);
        check = 0;
      }

      // Send next task to the slave
      if (npxc < farm_res){
       
        qbeforeS = load_sym(handler, md, "master_beforeSend", MECHANIC_MODULE_SILENT);
        if(qbeforeS) mstat = qbeforeS(mpi_status.MPI_SOURCE, md, d, rawdata);
         
        MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, mpi_status.MPI_SOURCE, MECHANIC_MPI_DATA_TAG, MPI_COMM_WORLD);
        npxc++;
        count++;
        
        qafterS = load_sym(handler, md, "master_afterSend", MECHANIC_MODULE_SILENT);
        if(qafterS) mstat = qafterS(mpi_status.MPI_SOURCE, md, d, rawdata);

      } else {
        break;
      }
    }

    /*
     * No more work to do, receive the outstanding results from the slaves.
     * We've got exactly 'count' messages to receive. 
     */
    for (i = 0; i < count; i++){
      
      qbeforeR = load_sym(handler, md, "master_beforeReceive", MECHANIC_MODULE_SILENT);
      if(qbeforeR) mstat = qbeforeR(md, d, rawdata);
        
      MPI_Recv(rawdata, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
      
      qafterR = load_sym(handler, md, "master_afterReceive", MECHANIC_MODULE_SILENT);
      if(qafterR) mstat = qafterR(mpi_status.MPI_SOURCE, md, d, rawdata);
     
      // Copy data to checkpoint arrays
      coordsarr[check][0] = rawdata->coords[0];
      coordsarr[check][1] = rawdata->coords[1];
      coordsarr[check][2] = rawdata->coords[2];
      
      for(j = 0; j < md->mrl; j++){
        resultarr[check][j] = rawdata->res[j];
      }
      check++;
    }
        
    // Write outstanding results to file
    mstat = atCheckPoint(check, coordsarr, board, resultarr, md, d);

    // Now, terminate the slaves 
    for (i = 1; i < nodes; i++){
        MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, i, MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);
    }

    // FARM ENDS
    MPI_Type_free(&masterResultsType);
    
    // Master can do something useful after the computations.
    query = load_sym(handler, md, "masterOUT", MECHANIC_MODULE_SILENT);
    if(query) mstat = query(nodes, md, d, rawdata);

    free(rawdata);
 
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

