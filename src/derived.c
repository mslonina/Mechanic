#include "mpifarm.h"

/**
 * MPI DERIVED DATATYPES
 */

/**
 * Master result Send/Recv
 */
void buildMasterResultsType(int mrl, masterData* md, MPI_Datatype* masterResultsType_ptr){

  int block_lengths[2];
  MPI_Aint displacements[2];
  MPI_Datatype typelist[2];
  MPI_Aint addresses[3];

  typelist[0] = MPI_INT;
  typelist[1] = MPI_DOUBLE;

  block_lengths[0] = 3;
  block_lengths[1] = mrl;

  MPI_Address(md, &addresses[0]);
  MPI_Address(md->coords, &addresses[1]);
  MPI_Address(md->res, &addresses[2]);

  displacements[0] = addresses[1] - addresses[0];
  displacements[1] = addresses[2] - addresses[0];

  MPI_Type_struct(2, block_lengths, displacements, typelist, masterResultsType_ptr);
  MPI_Type_commit(masterResultsType_ptr);
}

/**
 * Bcast default config file
 */
void buildDefaultConfigType(configData* d, MPI_Datatype* defaultConfigType_ptr){
  
  int block_lengths[8];
  MPI_Aint displacements[8];
  MPI_Datatype typelist[8];
  MPI_Aint addresses[9];
  int i = 0;

  typelist[0] = MPI_CHAR; //problem name
  typelist[1] = MPI_CHAR; //master datafile
  typelist[2] = MPI_CHAR; //module
  typelist[3] = MPI_INT; //xres
  typelist[4] = MPI_INT; //yres
  typelist[5] = MPI_INT; //method
  typelist[6] = MPI_INT; //master result length
  typelist[7] = MPI_INT; //dump

  block_lengths[0] = 256;
  block_lengths[1] = 260;
  block_lengths[2] = 256;
  block_lengths[3] = 1;
  block_lengths[4] = 1;
  block_lengths[5] = 1;
  block_lengths[6] = 1;
  block_lengths[7] = 1;

  MPI_Address(d, &addresses[0]);
  MPI_Address(&(d->name), &addresses[1]);
  MPI_Address(&(d->datafile), &addresses[2]);
  MPI_Address(&(d->module), &addresses[3]);
  MPI_Address(&(d->xres), &addresses[4]);
  MPI_Address(&(d->yres), &addresses[5]);
  MPI_Address(&(d->method), &addresses[6]);
  MPI_Address(&(d->mrl), &addresses[7]);
  MPI_Address(&(d->dump), &addresses[8]);

  for(i = 0; i < 8; i++){
    displacements[i] = addresses[i+1] - addresses[0];
  }

  MPI_Type_struct(8, block_lengths, displacements, typelist, defaultConfigType_ptr);
  MPI_Type_commit(defaultConfigType_ptr);

}