#include "mpifarm.h"

/**
 * SLAVE
 *
 */
void slave(void* module, configData *d){

    int *tab=malloc(3*sizeof(*tab));
    int k = 0, i = 0, j = 0;
    
    MPI_Datatype masterResultsType;
    MPI_Status mpi_status;

    module_query_void_f qbeforeS, qafterS, qbeforeR, qafterR, qpx, qpc;
    
    masterData raw;
    masterData *rawdata;
    
    rawdata = malloc(sizeof(masterData) + (d->mrl-1)*sizeof(MY_DATATYPE));

    clearArray(rawdata->res,ITEMS_IN_ARRAY(rawdata->res));
   
    /**
     * Slave can do something useful before computations.
     */
    query = load_sym(module, "userdefined_slaveIN", MODULE_SILENT);
    if(query) query(mpi_rank, d, rawdata);

    qbeforeR = load_sym(module, "userdefined_slave_beforeReceive", MODULE_SILENT);
    if(qbeforeR) qbeforeR(mpi_rank, d, rawdata);
    
       MPI_Recv(tab, 3, MPI_INT, MPI_DEST, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
    
    qafterR = load_sym(module, "userdefined_slave_afterReceive", MODULE_SILENT);
    if(qafterR) qafterR(mpi_rank, d, rawdata);

    buildMasterResultsType(d->mrl, rawdata, &masterResultsType);
    
    while(1){

     if(mpi_status.MPI_TAG == MPI_TERMINATE_TAG) break;     
     if(mpi_status.MPI_TAG == MPI_DATA_TAG){ 
      
       /**
        * One pixel per each slave.
        */
       if (d->method == 0){
          rawdata->coords[0] = tab[0];
          rawdata->coords[1] = tab[1];
          rawdata->coords[2] = tab[2];
       }
       if (d->method == 6){

          /**
           * Use userdefined pixelCoords method.
           */
          qpc = load_sym(module, "userdefined_pixelCoords", MODULE_ERROR);
          if(qpc) qpc(mpi_rank, tab, d, rawdata);
       } 
          /* PIXEL COMPUTATION */
          qpx = load_sym(module, "userdefined_pixelCompute", MODULE_ERROR);
          if(qpx) qpx(mpi_rank, d, rawdata);
          
          qbeforeS = load_sym(module, "userdefined_slave_beforeSend", MODULE_SILENT);
          if(qbeforeS) qbeforeS(mpi_rank, d, rawdata);
         
             MPI_Send(rawdata, 1, masterResultsType, MPI_DEST, MPI_RESULT_TAG, MPI_COMM_WORLD);

          qafterS = load_sym(module, "userdefined_slave_afterSend", MODULE_SILENT);
          if(qafterS) qafterS(mpi_rank, d, rawdata);
        
          qbeforeR = load_sym(module, "userdefined_slave_beforeReceive", MODULE_SILENT);
          if(qbeforeR) qbeforeR(mpi_rank, d, rawdata);
           
             MPI_Recv(tab, 3, MPI_INT, MPI_DEST, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
          
          qafterR = load_sym(module, "userdefined_slave_afterReceive", MODULE_SILENT);
          if(qafterR) qafterR(mpi_rank, d, rawdata);
        }
      
    }
    
    /**
     * Slave can do something useful after computations.
     */
    query = load_sym(module, "userdefined_slaveOUT", MODULE_SILENT);
    if(query) query(mpi_rank, d, rawdata);

    free(rawdata);
    return;
}

