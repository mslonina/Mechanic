/**
 * @file
 * Data storage and management (public API)
 */
#include "M2Spublic.h"

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
 * @brief Get the 1D size of the array
 *
 * @param rank The rank of the array
 * @param dims The dimensions of the array
 *
 * @return The 1D size of the array
 */
unsigned int GetSize(unsigned int rank, unsigned int *dims){
  unsigned int i = 0, size = 0;

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
void GetDims(storage *s, unsigned int *dims) {
  unsigned int i = 0;

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

  Message(MESSAGE_DEBUG, "Storage alloc: size = %zu, datatype = %zu\n", size, datatype);
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
 * @brief Allocates the memory buffer for attribute
 *
 * @param buffer The memory buffer to allocate
 * @param size The size of the memory buffer
 * @param datatype The size of the datatype
 *
 * @return SUCCESS on success, error code otherwise
 */
int AllocateAttribute(attr *s, size_t size, size_t datatype) {

  if (s->memory) {
    Message(MESSAGE_ERR, "The attribute buffer for '%s' is already allocated\n",
        s->layout.name);
    return CORE_ERR_MEM;
  }

  Message(MESSAGE_DEBUG, "Attr alloc: size = %zu, datatype = %zu\n", size, datatype);
  if (size > 0) {
    s->memory = calloc(size, datatype);
  }

  if (!s->memory) return CORE_ERR_MEM;
  return SUCCESS;
}

/**
 * @brief Free the memory buffer of the attribute
 *
 * @param s The storage object
 */
void FreeAttribute(attr *s) {
  if (s->memory) free(s->memory);
}

/**
 * @brief Commits the memory layout
 *
 * This function must run on every node -- the size of data arrays shared between nodes
 * depends on the memory layout.
 *
 * @param banks The number of memory/storage banks
 * @param s The storage structure
 *
 * @return 0 on success, error code otherwise
 *
 */
int CommitMemoryLayout(int banks, storage *s) {
  int mstat = SUCCESS, i = 0;

  for (i = 0; i < banks; i++) {
    mstat = Allocate(&s[i], s[i].layout.storage_elements, s[i].layout.datatype_size);
    CheckStatus(mstat);
  
    mstat = CommitAttrMemoryLayout(s[i].attr_banks, s);
    CheckStatus(mstat);
  }

  return mstat;
}

/**
 * @brief Commits the attribute memory layout
 *
 * @param banks The number of memory/storage banks
 * @param s The storage structure
 *
 * @return 0 on success, error code otherwise
 *
 */
int CommitAttrMemoryLayout(int banks, storage *s) {
  int mstat = SUCCESS, i = 0;

  Message(MESSAGE_DEBUG, "Storage '%s' attr_banks = %d\n", s->layout.name, s->attr_banks);
  for (i = 0; i < banks; i++) {
    Message(MESSAGE_DEBUG, "Attribute '%s', elements = %d, size = %zu\n",
        s->attr[i].layout.name, s->attr[i].layout.storage_elements, s->attr[i].layout.datatype_size);
    mstat = AllocateAttribute(&s->attr[i], s->attr[i].layout.storage_elements, s->attr[i].layout.datatype_size);
    CheckStatus(mstat);
  }

  return mstat;
}

/**
 * @brief Frees the memory layout
 *
 * @param banks The number of memory/storage banks
 * @param s The storage structure to free
 */
void FreeMemoryLayout(int banks, storage *s) {
  int i = 0, j = 0;

  for (i = 0; i < banks; i++) {
    if (s[i].layout.rank > 0) {
      for (j = 0; j < s[i].attr_banks; j++) {
        FreeAttribute(&s[i].attr[j]);
      }
      if (s[i].memory) {
        Free(&s[i]);
      }
    }
  }
}

/**
 * @brief Commit the data to the master file
 *
 * @param h5location The HDF5 location id
 * @param banks Number of memory banks to store
 * @param s The storage structure
 *
 * @return 0 on success, error code otherwise
 */
int CommitData(hid_t h5location, int banks, storage *s) {
  int mstat = SUCCESS, i = 0, j = 0;
  hid_t dataspace, dataset, memspace;
  herr_t hdf_status = 0;
  hsize_t dims[MAX_RANK], offsets[MAX_RANK];
  char *buffer = NULL;

  for (i = 0; i < banks; i++) {
    if (s[i].layout.use_hdf && s[i].layout.size > 0) {

      dataset = H5Dopen2(h5location, s[i].layout.name, H5P_DEFAULT);
      H5CheckStatus(dataset);
      dataspace = H5Dget_space(dataset);
      H5CheckStatus(dataspace);

      buffer = calloc(s[i].layout.elements, s[i].layout.datatype_size);
      ReadData(&s[i], buffer);

      /* Whole dataset at once */
      if (s[i].layout.storage_type == STORAGE_GROUP) {

        hdf_status = H5Dwrite(dataset, s[i].layout.datatype,
            H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
        H5CheckStatus(hdf_status);

      } else {
        for (j = 0; j < MAX_RANK; j++) {
          dims[j] = s[i].layout.storage_dim[j];
          offsets[j] = s[i].layout.offsets[j];
        }

        memspace = H5Screate_simple(s[i].layout.rank, dims, NULL);
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offsets, NULL, dims, NULL);
        hdf_status = H5Dwrite(dataset, s[i].layout.datatype,
            memspace, dataspace, H5P_DEFAULT, buffer);
        H5CheckStatus(hdf_status);

        H5Sclose(memspace);
      }

      if (buffer) free(buffer);

      H5Sclose(dataspace);
      H5Dclose(dataset);

    }
  }

  return mstat;
}

/**
 * @brief Commits the attribute to the HDF5 location
 *
 * @param h5location The HDF5 location
 * @param a The attribtue to store
 *
 */
int CommitAttribute(hid_t h5location, attr *a) {
  int mstat = SUCCESS, i = 0;
  hid_t attr_s, attr_d, ctype, memtype;
  herr_t h5status = 0;
  hsize_t dims[MAX_RANK], sdims[1] = {1};
  char *buffer = NULL;
      
  buffer = calloc(a->layout.elements, a->layout.datatype_size);
  ReadAttr(a, buffer);

  if (H5Aexists(h5location, a->layout.name) > 0) {
    if (a->layout.datatype == H5T_NATIVE_CHAR || a->layout.datatype == H5T_C_S1) {
      
      /**
       * Existing string attributes
       */
      Message(MESSAGE_DEBUG, "String attribute '%s' committed: %s\n", a->layout.name, a->memory);
      
      ctype = H5Tcopy(H5T_C_S1);
      h5status = H5Tset_size(ctype, CONFIG_LEN);
      memtype = H5Tcopy(H5T_C_S1);
      h5status = H5Tset_size(memtype, CONFIG_LEN);
      attr_s = H5Screate_simple(1, sdims, NULL);
      
      attr_d = H5Aopen(h5location, a->layout.name, H5P_DEFAULT);
      H5CheckStatus(attr_d);
      H5Awrite(attr_d, memtype, buffer);
      
      H5Sclose(attr_s);
      H5Aclose(attr_d);
      H5Tclose(ctype);
      H5Tclose(memtype);

    } else {

      /**
       * Existing numeric attributes
       */
      attr_d = H5Aopen(h5location, a->layout.name, H5P_DEFAULT);
      H5Awrite(attr_d, a->layout.datatype, buffer); 
      H5Aclose(attr_d);
    }
  } else {
    if (a->layout.datatype == H5T_NATIVE_CHAR || a->layout.datatype == H5T_C_S1) {

      /**
       * String attributes
       */
      Message(MESSAGE_DEBUG, "String attribute '%s' created: %s\n", a->layout.name, a->memory);
      
      ctype = H5Tcopy(H5T_C_S1);
      h5status = H5Tset_size(ctype, CONFIG_LEN);
      memtype = H5Tcopy(H5T_C_S1);
      h5status = H5Tset_size(memtype, CONFIG_LEN);
      attr_s = H5Screate_simple(1, sdims, NULL);

      attr_d = H5Acreate2(h5location, a->layout.name, memtype, attr_s, H5P_DEFAULT, H5P_DEFAULT);
      H5CheckStatus(attr_d);
      H5Awrite(attr_d, memtype, buffer);
      
      H5Aclose(attr_d);
      H5Sclose(attr_s);
      H5Tclose(ctype);
      H5Tclose(memtype);
    } else {
     
      /**
       * Numeric attributes
       */
      attr_s = H5Screate(a->layout.dataspace);
      H5CheckStatus(attr_s);

      if (a->layout.dataspace == H5S_SIMPLE) {
        for (i = 0; i < MAX_RANK; i++) {
          dims[i] = a->layout.storage_dim[i];
        }
      
        h5status = H5Sset_extent_simple(attr_s, a->layout.rank, dims, NULL);
        H5CheckStatus(h5status);
      }
    
      attr_d = H5Acreate2(h5location, a->layout.name, a->layout.datatype, attr_s, H5P_DEFAULT, H5P_DEFAULT);
      H5CheckStatus(attr_d);
      
      H5Awrite(attr_d, a->layout.datatype, buffer); 
      H5Sclose(attr_s);
      H5Aclose(attr_d);
    }
  }

  if (buffer) free(buffer);

  return mstat;
}

/**
 * @brief Read the dataset data
 *
 * @param h5location The HDF5 location id
 * @param banks Number of memory banks to read
 * @param s The storage structure
 *
 * @return 0 on success, error code otherwise
 */
int ReadDataset(hid_t h5location, int banks, storage *s, int size) {
  int mstat = SUCCESS, i = 0;
  int elements;
  void *buffer = NULL;
  hid_t dataset;
  herr_t hstat;

  for (i = 0; i < banks; i++) {
    if (s[i].layout.use_hdf) {
      elements = s[i].layout.elements;
      if (size > 1) {
        elements *= size;
      }
      buffer = calloc(elements, s[i].layout.datatype_size);

      Message(MESSAGE_DEBUG, "Read Data storage name: %s\n", s[i].layout.name);
      dataset = H5Dopen2(h5location, s[i].layout.name, H5P_DEFAULT);
      hstat = H5Dread(dataset, s[i].layout.datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
      H5CheckStatus(hstat);
      H5Dclose(dataset);

      WriteData(&s[i], buffer);
      free(buffer);
    }
  }

  return mstat;
}

