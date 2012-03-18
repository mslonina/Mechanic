/**
 * @file
 * Common functions.
 */

#include "MCommon.h"

/**
 * @function
 * Creates name
 *
 * @in_group
 * Helpers
 */
char* Name(char *prefix, char* name, char *suffix, char *extension) {
  char* fname;
  size_t preflen, nlen, sufflen, extlen, filelen;

  preflen = strlen(prefix);
  nlen = strlen(name);
  sufflen = strlen(suffix);
  extlen = strlen(extension);

  filelen = preflen + nlen + sufflen + extlen + 1;

  fname = calloc(filelen*sizeof(char*), sizeof(char*));
  if (!fname) Error(CORE_ERR_MEM);

  strncpy(fname, prefix, preflen);
  fname[preflen] = LRC_NULL;

  strncat(fname, name, nlen);
  fname[preflen+nlen] = LRC_NULL;

  strncat(fname, suffix, sufflen);
  fname[preflen+nlen+sufflen] = LRC_NULL;

  strncat(fname, extension, extlen);
  fname[filelen] = LRC_NULL;
  
  return fname;
}

/**
 * @function
 * Common messaging interface
 */
void Message(int type, char *message, ...) {

  int silent = 0;
  int debug = 0;
  static char message2[2048];
  va_list args;

  va_start(args, message);
    vsprintf(message2, message, args);
    if (silent == 0) {
      if (type == MESSAGE_INFO) printf("-> %s", message2);
      if (type == MESSAGE_CONT) printf("   %s", message2);
      if (type == MESSAGE_CONT2) printf("   \t\t %s", message2);
		  if (type == MESSAGE_DEBUG && debug == 1) printf("   %s", message2);
    }
      if (type == MESSAGE_ERR) perror(message2);
      if (type == MESSAGE_IERR) printf("!! %s", message2);
		  if (type == MESSAGE_WARN) printf(".. %s", message2);
  va_end(args);

}

/**
 * @function
 * Common error handler
 */
void Error(int errcode) {
  Abort(errcode);
}

/**
 * @function
 * Wrapper to MPI_Abort
 */
void Abort(int errcode) {
  MPI_Abort(MPI_COMM_WORLD, errcode);
}

/**
 * @function
 * Common status check
 */
void CheckStatus(int status) {
  if (status >= CORE_ERR_CORE) Error(status); 
}

/**
 * @function
 * Allocates double vector
 */
double* AllocateDoubleVec(int *dims) {
  double *vec;

//  printf("Allocation VEC = %d\n", dims[0]);
  vec = calloc(sizeof(double*) * dims[0], sizeof(double*));
  if (!vec) Error(CORE_ERR_MEM);

  return vec;
}

/** 
 * @function
 * Allocates double array
 *
 * see http://www.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_writedyn.c
 * see http://stackoverflow.com/questions/5104847/mpi-bcast-a-dynamic-2d-array
 */
double** AllocateDoubleArray(int rank, int *dims) {
  double** array;
  int i, size;

  size = GetSize(rank, dims);

  array = (double**) calloc(dims[0]*sizeof(double*), sizeof(double*));
  if (array) {
    array[0] = (double*) calloc(size*sizeof(double), sizeof(double));
    for (i = 1; i < dims[0]; i++) array[i] = array[0] + i*dims[1];
  }

  return array;
}

/**
 * @function
 * Frees double vector
 */
void FreeDoubleVec(double *vec, int *dims) {
  free(vec);
}

/**
 * @function
 * Frees double array
 *
 * see http://www.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_writedyn.c
 */
void FreeDoubleArray(double **array, int *dims) {

  if (array[0]) free(array[0]);
  if (array) free(array);

}

/**
 * @function
 * Gets the size.
 */
int GetSize(int rank, int *dims){
  int i = 0, size;

  size = dims[0];
  for (i = 1; i < rank; i++) {
    size = size * dims[i];
  }

  return size;
}

/**
 * @function
 * Copies Array to vector
 */
void Array2Vec(double *vec, double **array, int rank, int *dims) {
  int i = 0, j = 0, k = 0;
 
  for (i = 0; i < dims[0]; i++) {
    k = i * dims[1];
    for (j = 0; j < dims[1]; j++) {
      vec[j+k] = array[i][j];
    }
  }
}

/**
 * @function
 * Copies vector to array
 */
void Vec2Array(double *vec, double **array, int rank, int *dims) {
  int i = 0, j = 0, k = 0;

  for (i = 0; i < dims[0]; i++) {
    k = i * dims[1];
    for (j = 0; j < dims[1]; j++) {
      array[i][j] = vec[j+k];
    }
  }
}
