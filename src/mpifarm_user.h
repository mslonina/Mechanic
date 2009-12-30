#ifndef MPIFARM_USER_H
#define MPIFARM_USER_H

//#undef MAX_RESULT_LENGTH
//#define MAX_RESULT_LENGTH 12

#undef MY_DATATYPE
#define MY_DATATYPE double

#undef MY_MPI_DATATYPE
#define MY_MPI_DATATYPE MPI_DOUBLE


typedef struct inputData_t {
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
} inputData_t;

inputData_t d;

#endif
