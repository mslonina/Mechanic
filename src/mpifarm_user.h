#ifndef MPIFARM_USER_H
#define MPIFARM_USER_H

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
