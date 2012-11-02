/**
 * @file
 * The Mechanic2 public interface
 */
#include "Mechanic2.h"

/**
 * Generic-type macro for 2D allocation
 *
 * @brief Allocate 2D memory buffer
 *
 * see http://www.hdfgroup.org/ftp/HDF5/examples/misc-examples/h5_writedyn.c
 * see http://stackoverflow.com/questions/5104847/mpi-bcast-a-dynamic-2d-array
 *
 * @param s The storage object for which the array has to be allocated
 *
 * @return Allocated array, NULL otherwise
 */
#define ALLOCATE2D(y,x) \
  x** y(storage *s) { \
    x** array = NULL; \
    int i = 0; \
    if (s->layout.storage_size > 0) { \
      array = calloc(s->layout.storage_dim[0], sizeof(x*)); \
      if (array) { \
        array[0] = calloc(s->layout.storage_size, sizeof(x)); \
        if (array[0]) { \
          for (i = 0; i < s->layout.storage_dim[0]; i++) { \
            array[i] = array[0] + i * s->layout.storage_dim[1]; \
          } \
        } else { \
          Error(CORE_ERR_MEM); \
        } \
      } else { \
        Error(CORE_ERR_MEM); \
      } \
    } \
    return array; \
  } \

#define FREE2D(y,x)\
  void y(x **array) {\
  if (array[0]) free(array[0]);\
  if (array) free(array);\
}\

ALLOCATE2D(AllocateInt2D,int);
ALLOCATE2D(AllocateFloat2D,float);
ALLOCATE2D(AllocateDouble2D,double);

FREE2D(FreeInt2D,int);
FREE2D(FreeFloat2D,float);
FREE2D(FreeDouble2D,double);

/**
 * Generic-type macro for 3D memory allocation
 *
 * @brief Allocate 3D memory buffer
 *
 * http://stackoverflow.com/questions/2306172/malloc-a-3-dimensional-array-in-c
 *
 * @param s The storage object for which the array has to be allocated
 *
 * @return Allocated array, NULL otherwise
 */
#define ALLOCATE3D(y,x)\
  x*** y(storage *s) {\
    x*** array = NULL;\
    int i = 0, j = 0;\
    int dim0, dim1, dim2;\
    dim0 = s->layout.storage_dim[0];\
    dim1 = s->layout.storage_dim[1];\
    dim2 = s->layout.storage_dim[2];\
    if (s->layout.storage_size > 0) {\
      array = malloc((dim0 * sizeof(x*)) + (dim0*dim1 * sizeof(x**)) + (dim0*dim1*dim2 * sizeof(x)));\
      if (array) {\
        for(i = 0; i < dim0; i++) {\
          array[i] = (x**)(array + dim0) + i * dim1;\
          for(j = 0; j < dim1; j++) {\
            array[i][j] = (x*)(array + dim0 + dim0*dim1) + i*dim1*dim2 + j*dim2;\
          }\
        }\
      } else {\
        Error(CORE_ERR_MEM);\
      }\
    }\
    return array;\
  }\

#define FREE3D(y,x)\
  void y(x ***array) {\
    free(array);\
  }\

ALLOCATE3D(AllocateInt3D,int);
ALLOCATE3D(AllocateFloat3D,float);
ALLOCATE3D(AllocateDouble3D,double);

FREE3D(FreeInt3D,int);
FREE3D(FreeFloat3D,float);
FREE3D(FreeDouble3D,double);

/**
 * @brief Common error handler
 * 
 * We abort on any error code
 * @param errcode The error code to use
 */
void Error(int errcode) {
  Abort(errcode);
}

/**
 * @brief Wrapper to MPI_Abort
 *
 * @param errcode The error code to use
 */
void Abort(int errcode) {
  MPI_Abort(MPI_COMM_WORLD, errcode);
}

/**
 * @brief Common status check
 *
 * Throw an exception on any error code
 *
 * @param status The status code to check for
 */
void CheckStatus(int status) {
  if (status >= MODULE_ERR_CORE && status <= CORE_ERR_OTHER) Error(status);
}

/**
 * @brief HDF5 status check
 *
 * @param status The status code to check for
 */
void H5CheckStatus(hid_t status) {
  if (status < 0) Error(CORE_ERR_HDF);
}

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
 * @brief Get dimensions of the storage object
 *
 * The result array should be of rank 1 and length MAX_RANK
 *
 * @param s The storage object
 * @param dims The result array
 */
void GetDims(storage *s, int *dims) {
  int i = 0;

  for (i = 0; i < s->layout.rank; i++) {
    dims[i] = s->layout.storage_dim[i];
  }
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

