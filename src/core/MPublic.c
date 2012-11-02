/**
 * @file
 * The Mechanic2 public interface
 */
#include "Mechanic2.h"

/**
 * @brief Common messaging interface
 *
 * @param type The type of the message
 * @param message The message to display
 */
void Message(int type, char *message, ...) {
  static char message2[2048];
  va_list args;

  va_start(args, message);
    vsprintf(message2, message, args);
    if (type == MESSAGE_INFO)    printf("-- %s", message2);
    if (type == MESSAGE_COMMENT) printf("#  %s", message2);
    if (type == MESSAGE_OUTPUT)  printf("   %s", message2);
    if (type == MESSAGE_RESULT)  printf(" > %s", message2);
    if (type == MESSAGE_ERR)     printf("!! %s", message2);
		if (type == MESSAGE_WARN)    printf(".. %s", message2);
#ifdef WITH_DEBUG
    if (type == MESSAGE_DEBUG)   printf("%s", message2);
#endif
  va_end(args);
}

/**
 * @brief Prints the dataset
 *
 * @param type The output prefix type
 * @param dataset The HDF5 dataset handler
 *
 * @todo Do it better (i.e. for other datatypes)
 */
void PrintDataset(int type, hid_t dataset) {
  int dims[MAX_RANK];
  int i, j;
  double **buffer;
  hid_t dataspace, datatype;
  hsize_t dimsf[2], maxdims[2];

  dataspace = H5Dget_space(dataset);
  datatype = H5Dget_type(dataset);
  H5Sget_simple_extent_dims(dataspace, dimsf, maxdims);

  dims[0] = (int)dimsf[0];
  dims[1] = (int)dimsf[1];

  buffer = AllocateDouble2D(2, dims);
  H5Dread(dataset, datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, &buffer[0][0]);

  // @todo do it better
  for (i = 0; i < dims[0]; i++) {
    for (j = 0; j < dims[1]; j++) {
      Message(MESSAGE_OUTPUT, "%f ", buffer[i][j]);
    }
    Message(MESSAGE_OUTPUT, "\n");
  }

  FreeBuffer(buffer);

  H5Sclose(dataspace);
}

/**
 * @brief Get the 1D size of the array
 *
 * @param rank The rank of the array
 * @param dims The dimensions of the array
 *
 * @return The 1D size of the array
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
 * @brief Allocate 2D double-size memory buffer
 *
 * see http://www.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_writedyn.c
 * see http://stackoverflow.com/questions/5104847/mpi-bcast-a-dynamic-2d-array
 *
 * @param rank The rank of the array to allocate
 * @param dims The dimensions of the array to allocate
 *
 * @return Allocated array, NULL otherwise
 */
double** AllocateDouble2D(int rank, int *dims) {
  double** array = NULL;
  int i = 0, size = 0;

  size = GetSize(rank, dims);

  if (size > 0) {
    array = calloc(dims[0], sizeof(double*));
    if (array) {
      array[0] = calloc(size, sizeof(double));
      for (i = 0; i < dims[0]; i++) array[i] = array[0] + i*dims[1];
    }
  }

  return array;
}

/**
 * @brief Allocate 2D integer-size memory buffer
 *
 * see http://www.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_writedyn.c
 * see http://stackoverflow.com/questions/5104847/mpi-bcast-a-dynamic-2d-array
 *
 * @param rank The rank of the array to allocate
 * @param dims The dimensions of the array to allocate
 *
 * @return Allocated array, NULL otherwise
 */
int** AllocateInt2D(int rank, int *dims) {
  int** array = NULL;
  int i = 0, size = 0;

  size = GetSize(rank, dims);

  if (size > 0) {
    array = calloc(dims[0], sizeof(int*));
    if (array) {
      array[0] = calloc(size, sizeof(int));
      for (i = 0; i < dims[0]; i++) array[i] = array[0] + i*dims[1];
    }
  }

  return array;
}

/**
 * @brief Free 2D double memory buffer
 *
 * see http://www.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_writedyn.c
 *
 * @param array The array pointer to free
 */
void FreeBuffer(double **array) {
  if (array[0]) free(array[0]);
  if (array) free(array);
}

/**
 * @brief Free 2D int memory buffer
 *
 * see http://www.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_writedyn.c
 *
 * @param array The array pointer to free
 */
void FreeIntBuffer(int **array) {
  if (array[0]) free(array[0]);
  if (array) free(array);
}

/**
 * @brief Allocates the memory buffer
 *
 * @param buffer The memory buffer to allocate
 * @param size The size of the memory buffer
 * @param datatype The size of the datatype
 *
 * @return SUCCESS on success, error code otherwise
 */
int Allocate(storage *s, size_t size, size_t datatype) {

  if (s->memory) {
    Message(MESSAGE_ERR, "The buffer is already allocated\n");
    return CORE_ERR_MEM;
  }

  if (size > 0) {
    s->memory = calloc(size, datatype);
  }

  if (!s->memory) return CORE_ERR_MEM;
  return SUCCESS;
}

/**
 * @brief Free the memory buffer
 *
 * @param s The storage object
 */
void Free(storage *s) {
  if (s->memory) free(s->memory);
}

/**
 * @brief Copies data from local buffer to the memory buffer
 *
 * @param s The storage object
 * @param data The data pointer
 *
 * @return SUCCESS on success, error code otherwise
 */
int WriteData(storage *s, void *data) {
  if (!s->memory) return CORE_ERR_MEM;
  if (!data) return CORE_ERR_MEM;

  CopyData(data, s->memory, s->layout.storage_size);
  return SUCCESS;
}

/**
 * @brief Copies data from the memory buffer to the local storage
 *
 * @param s The storage object
 * @param data The data pointer
 *
 * @return SUCCESS on success, error code otherwise
 */
int ReadData(storage *s, void *data) {
  if (!s->memory) return CORE_ERR_MEM;
  if (!data) return CORE_ERR_MEM;

  CopyData(s->memory, data, s->layout.storage_size);
  return SUCCESS;
}

/**
 * @brief Wrapper to memcpy
 *
 * @param in The input buffer
 * @param out The output buffer
 * @param size The size of the block to be copied
 *
 * @return SUCCESS on success, error code otherwise
 */
int CopyData(void *in, void *out, size_t size) {
  if (!in) return CORE_ERR_MEM;
  if (!out) return CORE_ERR_MEM;

  memcpy(out, in, size);
  return SUCCESS;
}

