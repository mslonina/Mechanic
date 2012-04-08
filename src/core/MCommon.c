/**
 * @file
 * Common functions
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

  static char message2[2048];
  va_list args;

  va_start(args, message);
    vsprintf(message2, message, args);
    if (type == MESSAGE_INFO) printf("-> %s", message2);
    if (type == MESSAGE_CONT) printf("   %s", message2);
    if (type == MESSAGE_CONT2) printf("   \t\t %s", message2);
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
 * Allocate memory buffer
 *
 * see http://www.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_writedyn.c
 * see http://stackoverflow.com/questions/5104847/mpi-bcast-a-dynamic-2d-array
 */
double** AllocateBuffer(int rank, int *dims) {
  double** array = NULL;
  int i = 0, size = 0;

  size = GetSize(rank, dims);

  if (size > 0) {
    array = calloc(dims[0]*sizeof(double*), sizeof(double*));
    if (array) {
      array[0] = calloc(size*sizeof(double), sizeof(double));
      for (i = 0; i < dims[0]; i++) array[i] = array[0] + i*dims[1];
    }
  }

  return array;
}

/**
 * @function
 * Free memory buffer
 *
 * see http://www.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_writedyn.c
 */
void FreeBuffer(double **array) {

  if (array[0]) free(array[0]);
  if (array) free(array);

}

/**
 * @function
 * Gets the size
 */
int GetSize(int rank, int *dims){
  int i = 0, size = 0;

  size = dims[0];
  for (i = 1; i < rank; i++) {
    size = size * dims[i];
  }

  return size;
}

/**
 * @function
 * Copies array to vector
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

/**
 * @function
 * Copies files
 */
int Copy(char* in, char* out) {
  int input;
  int output;
  int mstat = 0;
  char *c;
  struct stat st;

  /* Read and write files in binary mode */
  input = open(in, O_RDONLY);
  if (input < 0) {
    Message(MESSAGE_ERR, "Could not open input file %s\n", in);
    return CORE_ERR_HDF;
  }

  stat(in, &st);
  Message(MESSAGE_DEBUG,
      "Input file size: %d\n", (int) st.st_size);

  output = open(out, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (output < 0) {
    Message(MESSAGE_ERR, "Could not open output file %s\n", out);
    return CORE_ERR_HDF;
  }

  /* CHAR_BIT in limits.h */
  c = malloc(st.st_size * CHAR_BIT + 1);
  if (c == NULL) return CORE_ERR_MEM;

  /* Read and write the whole file, without loops */
  mstat = read(input, c, st.st_size);
  if (mstat < 0) {
    Message(MESSAGE_ERR, "Could not read input file %s\n", in);
    return CORE_ERR_HDF;
  }

  mstat = write(output, c, st.st_size);
  if (mstat < 0) {
    Message(MESSAGE_ERR, "Could not write output file %s\n", out);
    return CORE_ERR_HDF;
  }

  free(c);

  if (close(input) < 0) {
    Message(MESSAGE_ERR, "Error closing input file\n");
    return CORE_ERR_HDF;
  }

  if (close(output) < 0) {
    Message(MESSAGE_ERR, "Error closing output file\n");
    return CORE_ERR_HDF;
  }

  return mstat;
}


