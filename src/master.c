#include "mpifarm.h"

/**
 * MASTER
 *
 */
void master(void* handler, moduleInfo* md, configData* d){

   int* tab; 
	 MY_DATATYPE rdata[d->mrl][1];
   int i = 0, k = 0, j = 0, nodes = 0, farm_res = 0;
   int npxc = 0; //number of all sent pixels
   int count = 0; //number of pixels to receive
   
   masterData raw;
   masterData *rawdata;

   MPI_Status mpi_status;
   MPI_Datatype masterResultsType;
   
   module_query_void_f queryIN, queryOUT, qbeforeS, qafterS, qbeforeR, qafterR;
   module_query_int_f qr;
 
   /* hdf */
   hid_t file_id, dset_board, dset_data, data_group;
   hid_t mapspace, memmapspace, rawspace, memrawspace, maprawspace; 
   hid_t plist_id;
   hsize_t dimsf[2], dimsr[2], co[2], rco[2], off[2], stride[2];
   int fdata[1][1];
   herr_t hdf_status;

   /**
    * Allocate memory for rawdata.res array.
    */
   rawdata = malloc(sizeof(masterData) + (d->mrl-1)*sizeof(MY_DATATYPE));

   clearArray(rawdata->res,ITEMS_IN_ARRAY(rawdata->res));
  
   /* Build derived type for master result */
   buildMasterResultsType(d->mrl, rawdata, &masterResultsType);
   
   /**
    * Master can do something useful before computations.
    */
   query = load_sym(handler, md, "masterIN", MODULE_SILENT);
   if(query) query(mpi_size, d);

   /**
    * Align farm resolution for given method.
    */
   if (d->method == 0) farm_res = d->xres*d->yres; //one pixel per each slave
   if (d->method == 1) farm_res = d->xres; //sliceX
   if (d->method == 2) farm_res = d->yres; //sliceY
   if (d->method == 6){
     qr = load_sym(handler, md, "farmResolution", MODULE_ERROR);
     if(qr) farm_res = qr(d->xres, d->yres);
   }

   /**
    * HDF5 Storage
    *
    * We write data in the following scheme:
    * /config -- configuration file
    * /board -- map of computed pixels
    * /data -- output data group
    * /data/master -- master dataset
    *
    * Each slave can write own data files if needed.
    * In such case, please edit slaveIN/OUT functions in your module.
    *
    */
 
        /* Open file */
    file_id = H5Fopen(d->datafile,H5F_ACC_RDWR,H5P_DEFAULT);
    
    /* Control board space */
    dimsf[0] = d->xres;
    dimsf[1] = d->yres;
    mapspace = H5Screate_simple(HDF_RANK, dimsf, NULL);
    
    dset_board = H5Dcreate(file_id, DATABOARD, H5T_NATIVE_INT, mapspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    /* Master data group */
    data_group = H5Gcreate(file_id, DATAGROUP, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    
    /* Result data space  */
    dimsr[0] = d->xres*d->yres;
    dimsr[1] = d->mrl;
    rawspace = H5Screate_simple(HDF_RANK, dimsr, NULL);

    /* Create master dataset */
    dset_data = H5Dcreate(data_group, DATASETMASTER, H5T_NATIVE_DOUBLE, rawspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  
    /* We write pixels one by one */
    co[0] = 1;
    co[1] = 1;
    memmapspace = H5Screate_simple(HDF_RANK, co, NULL);

    rco[0] = 1;
    rco[1] = d->mrl;
    memrawspace = H5Screate_simple(HDF_RANK, rco, NULL);
   
    mapspace = H5Dget_space(dset_board);
    rawspace = H5Dget_space(dset_data);
  
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
     qbeforeS = load_sym(handler, md,"master_beforeSend", MODULE_SILENT);
     if(qbeforeS) qbeforeS(i,d,rawdata);

     MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, i, MPI_DATA_TAG, MPI_COMM_WORLD);
     
     qafterS = load_sym(handler, md, "master_afterSend", MODULE_SILENT);
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
      MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, i, MPI_TERMINATE_TAG, MPI_COMM_WORLD);
    }
   }

   /**
    * Receive data and send tasks.
    */
   while (1) {
    
     qbeforeR = load_sym(handler, md, "master_beforeReceive", MODULE_SILENT);
     if(qbeforeR) qbeforeR(d,rawdata);
      
     MPI_Recv(rawdata, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
     count--;
      
      qafterR = load_sym(handler, md, "master_afterReceive", MODULE_SILENT);
      if(qafterR) qafterR(mpi_status.MPI_SOURCE, d, rawdata);
      
      /**
       * WRITE RESULTS TO MASTER FILE
       */
      
      /* Control board -- each computed pixel is marked with 1. */
      H5writeBoard(dset_board, memmapspace, mapspace, rawdata);

      /* Data */
      H5writeMaster(dset_data, memrawspace, rawspace, d, rawdata);

      /* Send next task to the slave */
      if (npxc < farm_res){
       
        qbeforeS = load_sym(handler, md, "master_beforeSend", MODULE_SILENT);
        if(qbeforeS) qbeforeS(mpi_status.MPI_SOURCE, &d, rawdata);
         
        MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, mpi_status.MPI_SOURCE, MPI_DATA_TAG, MPI_COMM_WORLD);
        npxc++;
        count++;
        
        qafterS = load_sym(handler, md, "master_afterSend", MODULE_SILENT);
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
      
      qbeforeR = load_sym(handler, md, "master_beforeReceive", MODULE_SILENT);
      if(qbeforeR) qbeforeR(d, rawdata);
        
      MPI_Recv(rawdata, 1, masterResultsType, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
      
      qafterR = load_sym(handler, md, "master_afterReceive", MODULE_SILENT);
      if(qafterR) qafterR(mpi_status.MPI_SOURCE, d, rawdata);
        
        /**
         * Write outstanding results to file
         */
      
        /* Control board */
        H5writeBoard(dset_board, memmapspace, mapspace, rawdata);
     
        /* Data */
        H5writeMaster(dset_data, memrawspace, rawspace, d, rawdata);

    }

    /**
     * Now, terminate the slaves 
     */
    for (i = 1; i < nodes; i++){
        MPI_Send(map2d(npxc, handler, md, d), 3, MPI_INT, i, MPI_TERMINATE_TAG, MPI_COMM_WORLD);
    }

    /**
     * FARM ENDS
     */
   
    MPI_Type_free(&masterResultsType);
    
    /**
     * CLOSE HDF5 STORAGE
     */
    H5Dclose(dset_board);
    H5Dclose(dset_data);
    H5Sclose(mapspace);
    H5Sclose(memmapspace);
    H5Sclose(rawspace);
    H5Sclose(memrawspace);
    H5Gclose(data_group);
    H5Fclose(file_id);

    /**
     * Master can do something useful after the computations.
     */
    query = load_sym(handler, md,"masterOUT", MODULE_SILENT);
    if(query) query(nodes, d, rawdata);

    free(rawdata);
    return;
}

