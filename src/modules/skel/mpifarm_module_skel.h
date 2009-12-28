#ifndef MPIFARM_PLUGIN_SKEL_H
#define MPIFARM_PLUGIN_SKEL_H

#undef MAX_RESULT_LENGTH
#undef MY_DATATYPE
#undef MY_MPI_DATATYPE

#define MAX_RESULT_LENGTH 12
#define MY_DATATYPE double
#define MY_MPI_DATATYPE MPI_DOUBLE

typedef struct {
  char name[256];
  char datafile[260];
  int bodies;
  int dump;
  MY_DATATYPE period;
  MY_DATATYPE cmass;
  MY_DATATYPE el[28];
  int xres;
  int yres;
  int method;
} inputData;

inputData d;

typedef struct {
  MY_DATATYPE points[20];
} slaveData;

slaveData s;


#endif
