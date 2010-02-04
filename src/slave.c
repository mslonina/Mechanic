#include "mechanic.h"
#include "mechanic-internals.h"

/**
 * SLAVE
 *
 */
int slave(void* handler, moduleInfo* md, configData* d){

    int* tab=malloc(3*sizeof(*tab));
    int k = 0, i = 0, j = 0;
   
    int mstat;

    module_query_void_f qbeforeS, qafterS, qbeforeR, qafterR, qpx, qpc;
    
    masterData raw;
    masterData* rawdata;
    
    MPI_Datatype masterResultsType;
    MPI_Status mpi_status;

    /* Allocate memory for rawdata.res array */
    rawdata = malloc(sizeof(masterData) + (d->mrl-1)*sizeof(MECHANIC_DATATYPE));

    clearArray(rawdata->res,ITEMS_IN_ARRAY(rawdata->res));
    
    /* Build derived type for master result */
    mstat = buildMasterResultsType(d->mrl, rawdata, &masterResultsType);
   
    /**
     * Slave can do something useful before computations.
     */
    query = load_sym(handler, md, "slaveIN", MECHANIC_MODULE_SILENT);
    if(query) mstat = query(mpi_rank, d, rawdata);

    qbeforeR = load_sym(handler, md, "slave_beforeReceive", MECHANIC_MODULE_SILENT);
    if(qbeforeR) mstat = qbeforeR(mpi_rank, d, rawdata);
    
       MPI_Recv(tab, 3, MPI_INT, MECHANIC_MPI_DEST, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
    
    qafterR = load_sym(handler, md, "slave_afterReceive", MECHANIC_MODULE_SILENT);
    if(qafterR) mstat = qafterR(mpi_rank, d, rawdata);
    
    while(1){

     if(mpi_status.MPI_TAG == MECHANIC_MPI_TERMINATE_TAG) break;     
     if(mpi_status.MPI_TAG == MECHANIC_MPI_DATA_TAG){ 
      
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
          qpc = load_sym(handler, md, "pixelCoords", MECHANIC_MODULE_ERROR);
          if(qpc) mstat = qpc(mpi_rank, tab, d, rawdata);
       } 
          /* PIXEL COMPUTATION */
          qpx = load_sym(handler, md, "pixelCompute", MECHANIC_MODULE_ERROR);
          if(qpx) mstat = qpx(mpi_rank, d, rawdata);
          
          qbeforeS = load_sym(handler, md, "slave_beforeSend", MECHANIC_MODULE_SILENT);
          if(qbeforeS) mstat = qbeforeS(mpi_rank, d, rawdata);
         
             MPI_Send(rawdata, 1, masterResultsType, MECHANIC_MPI_DEST, MECHANIC_MPI_RESULT_TAG, MPI_COMM_WORLD);

          qafterS = load_sym(handler, md, "slave_afterSend", MECHANIC_MODULE_SILENT);
          if(qafterS) mstat = qafterS(mpi_rank, d, rawdata);
        
          qbeforeR = load_sym(handler, md, "slave_beforeReceive", MECHANIC_MODULE_SILENT);
          if(qbeforeR) mstat = qbeforeR(mpi_rank, d, rawdata);
           
             MPI_Recv(tab, 3, MPI_INT, MECHANIC_MPI_DEST, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
          
          qafterR = load_sym(handler, md, "slave_afterReceive", MECHANIC_MODULE_SILENT);
          if(qafterR) mstat = qafterR(mpi_rank, d, rawdata);
        }
      
    }
    
    /**
     * Slave can do something useful after computations.
     */
    query = load_sym(handler, md, "slaveOUT", MECHANIC_MODULE_SILENT);
    if(query) mstat = query(mpi_rank, d, rawdata);

    free(rawdata);
    return 0;
}

