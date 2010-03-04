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

/**
 * SLAVE
 *
 */
int mechanic_mode_farm_slave(int node, void* handler, moduleInfo* md, configData* d){

    int* tab=malloc(3*sizeof(*tab));
    int k = 0, i = 0, j = 0;
   
    int mstat;

    module_query_void_f qbeforeS, qafterS, qbeforeR, qafterR, qpx, qpc;
    
    masterData raw;
    masterData* rawdata;
    
    MPI_Datatype masterResultsType;
    MPI_Status mpi_status;

    /* Allocate memory for rawdata.res array */
    rawdata = malloc(sizeof(masterData) + (md->mrl-1)*sizeof(MECHANIC_DATATYPE));

    /* Build derived type for master result */
    mstat = buildMasterResultsType(md->mrl, rawdata, &masterResultsType);
   
    /**
     * Slave can do something useful before computations.
     */
    query = load_sym(handler, md, "slaveIN", MECHANIC_MODULE_SILENT);
    if(query) mstat = query(mpi_rank, md, d, rawdata);

    qbeforeR = load_sym(handler, md, "slave_beforeReceive", MECHANIC_MODULE_SILENT);
    if(qbeforeR) mstat = qbeforeR(mpi_rank, md, d, rawdata);
    
       MPI_Recv(tab, 3, MPI_INT, MECHANIC_MPI_DEST, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
    
    qafterR = load_sym(handler, md, "slave_afterReceive", MECHANIC_MODULE_SILENT);
    if(qafterR) mstat = qafterR(mpi_rank, md, d, rawdata);
    
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
          if(qpc) mstat = qpc(mpi_rank, tab, md, d, rawdata);
       } 
          /* PIXEL COMPUTATION */
          qpx = load_sym(handler, md, "pixelCompute", MECHANIC_MODULE_ERROR);
          if(qpx) mstat = qpx(mpi_rank, md, d, rawdata);
          
          qbeforeS = load_sym(handler, md, "slave_beforeSend", MECHANIC_MODULE_SILENT);
          if(qbeforeS) mstat = qbeforeS(mpi_rank, md, d, rawdata);
         
             MPI_Send(rawdata, 1, masterResultsType, MECHANIC_MPI_DEST, MECHANIC_MPI_RESULT_TAG, MPI_COMM_WORLD);

          qafterS = load_sym(handler, md, "slave_afterSend", MECHANIC_MODULE_SILENT);
          if(qafterS) mstat = qafterS(mpi_rank, md, d, rawdata);
        
          qbeforeR = load_sym(handler, md, "slave_beforeReceive", MECHANIC_MODULE_SILENT);
          if(qbeforeR) mstat = qbeforeR(mpi_rank, md, d, rawdata);
           
             MPI_Recv(tab, 3, MPI_INT, MECHANIC_MPI_DEST, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);
          
          qafterR = load_sym(handler, md, "slave_afterReceive", MECHANIC_MODULE_SILENT);
          if(qafterR) mstat = qafterR(mpi_rank, md, d, rawdata);
        }
      
    }
    
    /**
     * Slave can do something useful after computations.
     */
    query = load_sym(handler, md, "slaveOUT", MECHANIC_MODULE_SILENT);
    if(query) mstat = query(mpi_rank, md, d, rawdata);

    free(rawdata);
    return 0;
}

