#ifndef MECHANIC_H
#define MECHANIC_H

#define MECHANIC_ERR_MPI 911
#define MECHANIC_ERR_HDF 912
#define MECHANIC_ERR_MODULE 913
#define MECHANIC_ERR_SETUP 914
#define MECHANIC_ERR_OTHER 999

#define MECHANIC_HDF_RANK 2

#undef MECHANIC_DATATYPE
#define MECHANIC_DATATYPE double

#undef MECHANIC_MPI_DATATYPE
#define MECHANIC_MPI_DATATYPE MPI_DOUBLE

#define ITEMS_IN_ARRAY(x) sizeof(x)/sizeof(*(x))

/* MAIN CONFIG DATA */
typedef struct {
  char name[256];
  char datafile[260];
  char module[256];
  int xres;
  int yres;
  int method;
  int mrl;
  int checkpoint;
  int restartmode;
  int mode;
  int checkpoint_num;
} configData;


/**
 * MASTER DATA
 */
typedef struct {
 int coords[3]; //0 - x 1 - y 2 - number of the pixel
 MECHANIC_DATATYPE res[1];
} masterData;

/* Module info and handler */
typedef struct {
      const char *name;
      const char *author;
      const char *date;
      const char *version;
} moduleInfo;

#endif
