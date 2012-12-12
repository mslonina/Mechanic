/**
 * @file
 * The Mechanic public interface
 */
#include "mechanic.h"

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
#define ALLOCATE2(y,x)\
  x** y(storage *s) {\
    x** array = NULL;\
    int i = 0;\
    int dim0, dim1;\
    dim0 = s->layout.storage_dim[0];\
    dim1 = s->layout.storage_dim[1];\
    if (s->layout.storage_size > 0) { \
      array = malloc((dim0 * sizeof(x*)) + (dim0 * dim1 * sizeof(x)));\
      if (array) {\
        for (i = 0; i < dim0; i++) {\
          array[i] = (x*)(array + dim0) + i * dim1;\
        }\
      } else {\
        Error(CORE_ERR_MEM);\
      }\
    } \
    return array; \
  } \

ALLOCATE2(AllocateInt2,int)
ALLOCATE2(AllocateShort2,short)
ALLOCATE2(AllocateLong2,long)
ALLOCATE2(AllocateLLong2,long long)
ALLOCATE2(AllocateUInt2,unsigned int)
ALLOCATE2(AllocateUShort2,unsigned short)
ALLOCATE2(AllocateULLong2,unsigned long long)
ALLOCATE2(AllocateFloat2,float)
ALLOCATE2(AllocateDouble2,double)

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
#define ALLOCATE3(y,x)\
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
        for (i = 0; i < dim0; i++) {\
          array[i] = (x**)(array + dim0) + i * dim1;\
          for (j = 0; j < dim1; j++) {\
            array[i][j] = (x*)(array + dim0 + dim0*dim1) + i*dim1*dim2 + j*dim2;\
          }\
        }\
      } else {\
        Error(CORE_ERR_MEM);\
      }\
    }\
    return array;\
  }\

ALLOCATE3(AllocateInt3,int)
ALLOCATE3(AllocateShort3,short)
ALLOCATE3(AllocateLong3,long)
ALLOCATE3(AllocateLLong3,long long)
ALLOCATE3(AllocateUInt3,unsigned int)
ALLOCATE3(AllocateUShort3,unsigned short)
ALLOCATE3(AllocateULLong3,unsigned long long)
ALLOCATE3(AllocateFloat3,float)
ALLOCATE3(AllocateDouble3,double)

/**
 * Generic-type macro for 4D memory allocation
 *
 * @brief Allocate 4D memory buffer
 *
 * @param s The storage object for which the array has to be allocated
 *
 * @return Allocated array, NULL otherwise
 */
#define ALLOCATE4(y,x)\
  x**** y(storage *s) {\
    x**** array = NULL;\
    int i = 0, j = 0, k = 0;\
    int dim0, dim1, dim2, dim3;\
    dim0 = s->layout.storage_dim[0];\
    dim1 = s->layout.storage_dim[1];\
    dim2 = s->layout.storage_dim[2];\
    dim3 = s->layout.storage_dim[3];\
    if (s->layout.storage_size > 0) {\
      array = malloc((dim0 * sizeof(x*)) + (dim0*dim1 * sizeof(x**)) + (dim0*dim1*dim2 * sizeof(x***))\
          + (dim0*dim1*dim2*dim3 * sizeof(x)));\
      if (array) {\
        for (i = 0; i < dim0; i++) {\
          array[i] = (x***)(array + dim0) + i * dim1;\
          for (j = 0; j < dim1; j++) {\
            array[i][j] = (x**)(array + dim0 + dim0*dim1) + i*dim1*dim2 + j*dim2;\
            for (k = 0; k < dim2; k++) {\
              array[i][j][k] = (x*)(array + dim0 + dim0*dim1 + dim0*dim1*dim2) + i*dim1*dim2*dim3 + j*dim2*dim3 + k*dim3;\
            }\
          }\
        }\
      } else {\
        Error(CORE_ERR_MEM);\
      }\
    }\
    return array;\
  }\

ALLOCATE4(AllocateInt4,int)
ALLOCATE4(AllocateShort4,short)
ALLOCATE4(AllocateLong4,long)
ALLOCATE4(AllocateLLong4,long long)
ALLOCATE4(AllocateUInt4,unsigned int)
ALLOCATE4(AllocateUShort4,unsigned short)
ALLOCATE4(AllocateULLong4,unsigned long long)
ALLOCATE4(AllocateFloat4,float)
ALLOCATE4(AllocateDouble4,double)

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
 * @brief Copies data from local buffer to the attribute memory buffer
 *
 * @param s The attribute object
 * @param data The data pointer
 *
 * @return SUCCESS on success, error code otherwise
 */
int WriteAttr(attr *a, void *data) {
  if (!a->memory) return CORE_ERR_MEM;
  if (!data) return CORE_ERR_MEM;

  CopyData(data, a->memory, a->layout.storage_size);
  return SUCCESS;
}

/**
 * @brief Copies data from the attribute buffer to the local storage
 *
 * @param s The attribute object
 * @param data The data pointer
 *
 * @return SUCCESS on success, error code otherwise
 */
int ReadAttr(attr *a, void *data) {
  if (!a->memory) return CORE_ERR_MEM;
  if (!data) return CORE_ERR_MEM;

  CopyData(a->memory, data, a->layout.storage_size);
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

/**
 * @brief Get the storage bank index for a given name
 *
 * @param s The storage array
 * @param name The name to look for
 *
 * @return storage index when the storage bank is found, -1 otherwise
 */
int GetStorageIndex(storage *s, char *name) {
  int index = 0;

  if (!name) return -1;
  if (!s) return -1;

  while (s[index].layout.rank > 0) {
    if (s[index].layout.name) {
      if (strcmp(s[index].layout.name, name) == 0) return index;
      index++;
    }
  }

  return -1;
}

/**
 * @brief Get the attribute bank index for a given name
 *
 * @param s The attribute array
 * @param name The name to look for
 *
 * @return storage index when the storage bank is found, -1 otherwise
 */
int GetAttributeIndex(attr *a, char *name) {
  int index = 0;

  if (!name) return -1;
  if (!a) return -1;

  while (a[index].layout.rank > 0) {
    if (a[index].layout.name) {
      if (strcmp(a[index].layout.name, name) == 0) return index;
      index++;
    }
  }

  return -1;
}

