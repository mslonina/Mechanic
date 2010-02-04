#include "mechanic.h"
#include "mechanic-internals.h"

/**
 * MASTER
 *
 */
void master(void* handler, moduleInfo* md, configData* d, int restartmode){

   int* tab;
   int i = 0, k = 0, j = 0, nodes = 0, farm_res = 0;
   int npxc = 0; //number of all sent pixels
   int count = 0; //number of pixels to receive
   int check = 0;
   
   masterData *rawdata;

   /* Checkpoint storage */
   int **coordsarr;
   MECHANIC_DATATYPE **resultarr;
   
   /* Restart mode board */
   int** board;

   MPI_Status mpi_status;
   MPI_Datatype masterResultsType;
   
   module_query_void_f queryIN, queryOUT, qbeforeS, qafterS, qbeforeR, qafterR;
   module_query_int_f qr;
 
   /**
    * Allocate memory for rawdata.res array.
    */
   rawdata = malloc(sizeof(masterData) + (d->mrl-1)*sizeof(MECHANIC_DATATYPE));

   clearArray(rawdata->res, ITEMS_IN_ARRAY(rawdata->res));

   /**
    * Allocate memory for checkpoint mode.
    */
   coordsarr = malloc(sizeof(int*)*d->checkpoint);
   resultarr = malloc(sizeof(MECHANIC_DATATYPE*)*d->checkpoint);

   for(i = 0; i < d->checkpoint; i++){
     coordsarr[i] = malloc(sizeof(int*)*3);
     resultarr[i] = malloc(sizeof(MECHANIC_DATATYPE*)*d->mrl);
     clearArray(resultarr[i],ITEMS_IN_ARRAY(resultarr[i]));
   }
   
   /* Build derived type for master result */
   buildMasterResultsType(d->mrl, rawdata, &masterResultsType);
   
   /* Allocate memory for board */
   if(restartmode == 1){
    board = malloc(sizeof(int*)*d->xres);
    for(i = 0; i < d->xres; i++) 
      board[i] = malloc(sizeof(int*)*d->yres);
    
   }

   if(restartmode == 1) H5readBoard(d, board);
   /*printf("\n");
   for(i = 0; i < d->xres; i++){
    for(j = 0; j < d->yres; j++){
      printf(" %3d", board[i][j]);
    }
    printf("\n");
   }
   */



   /**
    * Master can do something useful before computations.
    */
   query = load_sym(handler, md, "masterIN", MECHANIC_MODULE_SILENT);
   if(query) query(mpi_size, d);

   /**
    * Align farm resolution for given method.
    */
   if (d->method == 0) farm_res = d->xres*d->yres; //one pixel per each slave
   if (d->method == 1) farm_res = d->xres; //sliceX
   if (d->method == 2) farm_res = d->yres; //sliceY
   if (d->method == 6){
     qr = load_sym(handler, md, "farmResolution", MECHANIC_MODULE_ERROR);
     if(qr) farm_res = qr(d->xres, d->yres);
   }
 
   /**
    * FARM STARTS
    */

   /**
    * Security check -- if farm resolution is greater than number of slaves.
    * Needed when we have i.e. 3 slaves and only one pixel to compute.
    */
   if (farm_res > mpi_size) nodes = mpi_size;
   if (farm_res == mpi_size) nodes = mpi_size;
   if (farm_res < mpi_size) nodes = farm_res + 1;

   /**
    * Send tasks to all slaves,
    * remembering what the farm resolution is.
    */
   for (i = 1; i < nodes; i++){
     qbeforeS = load_sym(handler, md,"master_beforeSend", MECHANIC_MODULE_SILENT);
     if(qbeforeS) qbeforeS(i,d,rawdata);

     MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, i, MECHANIC_MPI_DATA_TAG, MPI_COMM_WORLD);
     
     qafterS = load_sym(handler, md, "master_afterSend", MECHANIC_MODULE_SILENT);
     if(qafterS) qafterS(i,d,rawdata);
     
     count++;
     npxc++;
   }
   
   /**
    * We don't want to have slaves idle, so, if there are some idle slaves
    * terminate them (when farm resolution < mpi size).
    */
   if(farm_res < mpi_size){
    for(i = nodes; i < mpi_size; i++){
      printf("-> Terminating idle slave %d.\n", i);
      MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, i, MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);
    }
   }

   /**
    * Receive data and send tasks.
    */
   while (1) {
    
     qbeforeR = load_sym(handler, md, "master_beforeReceive", MECHANIC_MODULE_SILENT);
     if(qbeforeR) qbeforeR(d,rawdata);
      
     MPI_Recv(rawdata, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
     count--;
      
      qafterR = load_sym(handler, md, "master_afterReceive", MECHANIC_MODULE_SILENT);
      if(qafterR) qafterR(mpi_status.MPI_SOURCE, d, rawdata);
    
      /* Copy data to checkpoint arrays */
      coordsarr[check][0] = rawdata->coords[0];
      coordsarr[check][1] = rawdata->coords[1];
      coordsarr[check][2] = rawdata->coords[2];
      
      for(j = 0; j < d->mrl; j++){
        resultarr[check][j] = rawdata->res[j];
      }
      check++;

      /* We write data only on checkpoint:
       * it is more healthier for disk, but remember --
       * if you set up checkpoints very often and your simulations are
       * very fast, your disks will not be happy
       * */
      if(check % d->checkpoint == 0){//FIX ME: add UPS checks  
        H5writeCheckPoint(d, check, coordsarr, resultarr);
        for(i = 0; i < check; i++) clearArray(resultarr[i], ITEMS_IN_ARRAY(resultarr[i]));
        check = 0;
      }

      /* Send next task to the slave */
      if (npxc < farm_res){
       
        qbeforeS = load_sym(handler, md, "master_beforeSend", MECHANIC_MODULE_SILENT);
        if(qbeforeS) qbeforeS(mpi_status.MPI_SOURCE, &d, rawdata);
         
        MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, mpi_status.MPI_SOURCE, MECHANIC_MPI_DATA_TAG, MPI_COMM_WORLD);
        npxc++;
        count++;
        
        qafterS = load_sym(handler, md, "master_afterSend", MECHANIC_MODULE_SILENT);
        if(qafterS) qafterS(mpi_status.MPI_SOURCE, d, rawdata);

      } else {
        break;
      }
    }

    /** 
     * No more work to do, receive the outstanding results from the slaves.
     * We've got exactly 'count' messages to receive. 
     */
    for (i = 0; i < count; i++){
      
      qbeforeR = load_sym(handler, md, "master_beforeReceive", MECHANIC_MODULE_SILENT);
      if(qbeforeR) qbeforeR(d, rawdata);
        
      MPI_Recv(rawdata, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
      
      qafterR = load_sym(handler, md, "master_afterReceive", MECHANIC_MODULE_SILENT);
      if(qafterR) qafterR(mpi_status.MPI_SOURCE, d, rawdata);
     
      /* Copy data to checkpoint arrays */
      coordsarr[check][0] = rawdata->coords[0];
      coordsarr[check][1] = rawdata->coords[1];
      coordsarr[check][2] = rawdata->coords[2];
      
      for(j = 0; j < d->mrl; j++){
        resultarr[check][j] = rawdata->res[j];
      }
      check++;
    }
        
    /**
     * Write outstanding results to file
     */
    H5writeCheckPoint(d, check, coordsarr, resultarr);

    /**
     * Now, terminate the slaves 
     */
    for (i = 1; i < nodes; i++){
        MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, i, MECHANIC_MPI_TERMINATE_TAG, MPI_COMM_WORLD);
    }

    /**
     * FARM ENDS
     */
    MPI_Type_free(&masterResultsType);
    
    /**
     * Master can do something useful after the computations.
     */
    query = load_sym(handler, md, "masterOUT", MECHANIC_MODULE_SILENT);
    if(query) query(nodes, d, rawdata);

    /* Free memory allocations */
    for(i = 0; i < d->checkpoint; i++){
     free(coordsarr[i]);
     free(resultarr[i]);
    }

    free(coordsarr);
    free(resultarr);
    free(rawdata);
  
   if(restartmode == 1){
    for(i = 0; i < d->xres; i++) free(board[i]);
    free(board);
   }  
  
   
   return;
}

