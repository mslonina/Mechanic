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
    unsigned int i = 0;\
    unsigned int dim0, dim1;\
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

ALLOCATE2(AllocateInt2, int)
ALLOCATE2(AllocateShort2, short)
ALLOCATE2(AllocateLong2, long)
ALLOCATE2(AllocateLLong2, long long)
ALLOCATE2(AllocateUInt2, unsigned int)
ALLOCATE2(AllocateUShort2, unsigned short)
ALLOCATE2(AllocateULLong2, unsigned long long)
ALLOCATE2(AllocateFloat2, float)
ALLOCATE2(AllocateDouble2, double)

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
    unsigned int i = 0, j = 0;\
    unsigned int dim0, dim1, dim2;\
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

ALLOCATE3(AllocateInt3, int)
ALLOCATE3(AllocateShort3, short)
ALLOCATE3(AllocateLong3, long)
ALLOCATE3(AllocateLLong3, long long)
ALLOCATE3(AllocateUInt3, unsigned int)
ALLOCATE3(AllocateUShort3, unsigned short)
ALLOCATE3(AllocateULLong3, unsigned long long)
ALLOCATE3(AllocateFloat3, float)
ALLOCATE3(AllocateDouble3, double)

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
    unsigned int i = 0, j = 0, k = 0;\
    unsigned int dim0, dim1, dim2, dim3;\
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

ALLOCATE4(AllocateInt4, int)
ALLOCATE4(AllocateShort4, short)
ALLOCATE4(AllocateLong4, long)
ALLOCATE4(AllocateLLong4, long long)
ALLOCATE4(AllocateUInt4, unsigned int)
ALLOCATE4(AllocateUShort4, unsigned short)
ALLOCATE4(AllocateULLong4, unsigned long long)
ALLOCATE4(AllocateFloat4, float)
ALLOCATE4(AllocateDouble4, double)

/**
 * Generic-type macro for 5D memory allocation
 *
 * @brief Allocate 5D memory buffer
 *
 * @param s The storage object for which the array has to be allocated
 *
 * @return Allocated array, NULL otherwise
 */
#define ALLOCATE5(y,x)\
  x***** y(storage *s) {\
    x***** array = NULL;\
    unsigned int i = 0, j = 0, k = 0, l = 0;\
    unsigned int dim0, dim1, dim2, dim3, dim4;\
    dim0 = s->layout.storage_dim[0];\
    dim1 = s->layout.storage_dim[1];\
    dim2 = s->layout.storage_dim[2];\
    dim3 = s->layout.storage_dim[3];\
    dim4 = s->layout.storage_dim[4];\
    if (s->layout.storage_size > 0) {\
      array = malloc((dim0 * sizeof(x*)) + (dim0*dim1 * sizeof(x**)) + (dim0*dim1*dim2 * sizeof(x***))\
          + (dim0*dim1*dim2*dim3 * sizeof(x****)) + (dim0*dim1*dim2*dim3*dim4 * sizeof(x)));\
      if (array) {\
        for (i = 0; i < dim0; i++) {\
          array[i] = (x****)(array + dim0) + i * dim1;\
          for (j = 0; j < dim1; j++) {\
            array[i][j] = (x***)(array + dim0 + dim0*dim1) + i*dim1*dim2 + j*dim2;\
            for (k = 0; k < dim2; k++) {\
              array[i][j][k] = (x**)(array + dim0 + dim0*dim1 + dim0*dim1*dim2) + i*dim1*dim2*dim3 + j*dim2*dim3 + k*dim3;\
              for (l = 0; l < dim3; l++) {\
                array[i][j][k][l] = (x*)(array + dim0 + dim0*dim1 + dim0*dim1*dim2 + dim0*dim1*dim2*dim3) + \
                  i*dim1*dim2*dim3*dim4 + j*dim2*dim3*dim4 + k*dim3*dim4 + l*dim4;\
              }\
            }\
          }\
        }\
      } else {\
        Error(CORE_ERR_MEM);\
      }\
    }\
    return array;\
  }\

ALLOCATE5(AllocateInt5, int)
ALLOCATE5(AllocateShort5, short)
ALLOCATE5(AllocateLong5, long)
ALLOCATE5(AllocateLLong5, long long)
ALLOCATE5(AllocateUInt5, unsigned int)
ALLOCATE5(AllocateUShort5, unsigned short)
ALLOCATE5(AllocateULLong5, unsigned long long)
ALLOCATE5(AllocateFloat5, float)
ALLOCATE5(AllocateDouble5, double)

/**
 * @brief Get the 1D size of the array
 *
 * @param rank The rank of the array
 * @param dims The dimensions of the array
 *
 * @return The 1D size of the array
 */
unsigned int GetSize(unsigned int rank, unsigned int *dims){
  unsigned int i = 0;
  unsigned int size = 0;

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
  if (s->field) {
    free(s->field);
  }
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

    mstat = CommitAttrMemoryLayout(s[i].attr_banks, &s[i]);
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
      if (s[i].attr) {
        for (j = 0; j < s[i].attr_banks; j++) {
          FreeAttribute(&s[i].attr[j]);
        }
        free(s[i].attr);
      }
      if (s[i].field) {
        free(s[i].field);
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
  hid_t h5datatype;
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

      h5datatype = s[i].layout.datatype;

      if (s[i].layout.datatype == H5T_COMPOUND) {
        h5datatype = CommitFileDatatype(&s[i]);
        H5CheckStatus(h5datatype);
      }

      /* Whole dataset at once */
      if (s[i].layout.storage_type == STORAGE_GROUP) {

        hdf_status = H5Dwrite(dataset, h5datatype,
            H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
        H5CheckStatus(hdf_status);

      } else {
        for (j = 0; j < MAX_RANK; j++) {
          dims[j] = s[i].layout.storage_dim[j];
          offsets[j] = s[i].layout.offsets[j];
        }

        memspace = H5Screate_simple(s[i].layout.rank, dims, NULL);
        H5Sselect_hyperslab(dataspace, H5S_SELECT_SET, offsets, NULL, dims, NULL);
        hdf_status = H5Dwrite(dataset, h5datatype,
            memspace, dataspace, H5P_DEFAULT, buffer);
        H5CheckStatus(hdf_status);

        H5Sclose(memspace);
      }

      if (buffer) free(buffer);

      H5Sclose(dataspace);
      H5Dclose(dataset);

      if (s[i].layout.datatype == H5T_COMPOUND) H5Tclose(h5datatype);
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
  hid_t attr_s, attr_d, ctype, memtype, attr_t;
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
       * Existing numeric and compound attributes
       */
      attr_t = a->layout.datatype;
      if (a->layout.datatype == H5T_COMPOUND) {
        attr_t = CommitAttrFileDatatype(a);
        H5CheckStatus(attr_t);
      }

      attr_d = H5Aopen(h5location, a->layout.name, H5P_DEFAULT);
      H5Awrite(attr_d, attr_t, buffer);
      H5Aclose(attr_d);

      if (a->layout.datatype == H5T_COMPOUND) H5Tclose(attr_t);
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
       * Numeric and compound attributes
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

      attr_t = a->layout.datatype;

      if (a->layout.datatype == H5T_COMPOUND) {
        attr_t = CommitAttrFileDatatype(a);
        H5CheckStatus(attr_t);
      }

      attr_d = H5Acreate2(h5location, a->layout.name, attr_t, attr_s, H5P_DEFAULT, H5P_DEFAULT);
      H5CheckStatus(attr_d);

      H5Awrite(attr_d, attr_t, buffer);
      H5Sclose(attr_s);
      H5Aclose(attr_d);

      if (a->layout.datatype == H5T_COMPOUND) H5Tclose(attr_t);
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
int ReadDataset(hid_t h5location, int banks, storage *s, unsigned int size) {
  int mstat = SUCCESS, i = 0;
  unsigned int elements;
  void *buffer = NULL;
  hid_t dataset, h5datatype;
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

      if (s[i].layout.datatype == H5T_COMPOUND) {
        h5datatype = CommitFileDatatype(&s[i]);
        hstat = H5Dread(dataset, h5datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
        H5CheckStatus(hstat);

        H5Tclose(h5datatype);

      } else {
        hstat = H5Dread(dataset, s[i].layout.datatype, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
        H5CheckStatus(hstat);
      }

      H5Dclose(dataset);

      WriteData(&s[i], buffer);
      free(buffer);
    }
  }

  return mstat;
}

/**
 * @brief
 * Commit the compound datatype to the memory
 *
 * @param s The storage structure
 *
 * @return The committed datatype on success, CORE_ERR_HDF on failure
 */
hid_t CommitDatatype(storage *s) {
  hid_t h5datatype, type;
  hsize_t adims[MAX_RANK];
  herr_t h5status;
  unsigned int i = 0, j = 0;

  h5datatype = H5Tcreate(H5T_COMPOUND, s->layout.compound_size);
  for (i = 0; i < s->compound_fields; i++) {
    if (s->field[i].layout.datatype > 0) {
      if (s->field[i].layout.elements > 1) {
        for (j = 0; j < MAX_RANK; j++) {
          adims[j] = s->field[i].layout.dims[j];
        }
        type = H5Tarray_create(s->field[i].layout.datatype, s->field[i].layout.rank, adims);
        h5status = H5Tinsert(h5datatype, s->field[i].layout.name,
            s->field[i].layout.field_offset, type);
        H5CheckStatus(h5status);
        H5Tclose(type);
      } else {
        type = s->field[i].layout.datatype;
        h5status = H5Tinsert(h5datatype, s->field[i].layout.name,
            s->field[i].layout.field_offset, type);
        H5CheckStatus(h5status);
      }
    }
  }
  return h5datatype;
}

/**
 * @brief
 * Commit the compound datatype to the file
 *
 * @param s The storage structure
 *
 * @return The committed datatype on success, CORE_ERR_HDF on failure
 */
hid_t CommitFileDatatype(storage *s) {
  hid_t h5datatype, type;
  hsize_t adims[MAX_RANK];
  herr_t h5status;
  unsigned int i = 0, j = 0;
  size_t c_size = 0;

  h5datatype = H5Tcreate(H5T_COMPOUND, s->layout.compound_size);
  for (i = 0; i < s->compound_fields; i++) {
    if (s->field[i].layout.datatype > 0) {

      // Special handling for char datatype
      if (s->field[i].layout.datatype == H5T_NATIVE_CHAR ||
          s->field[i].layout.datatype == H5T_NATIVE_UCHAR) {
        type = H5Tcopy(H5T_C_S1);
        c_size = s->field[i].layout.elements;
        H5Tset_size(type, c_size);
        h5status = H5Tinsert(h5datatype, s->field[i].layout.name,
            s->field[i].layout.field_offset, type);
        H5CheckStatus(h5status);
        H5Tclose(type);

      } else {
        if (s->field[i].layout.elements > 1) {
          for (j = 0; j < MAX_RANK; j++) {
            adims[j] = s->field[i].layout.dims[j];
          }
          type = H5Tarray_create(s->field[i].layout.datatype, s->field[i].layout.rank, adims);
          h5status = H5Tinsert(h5datatype, s->field[i].layout.name,
              s->field[i].layout.field_offset, type);
          H5CheckStatus(h5status);
          H5Tclose(type);
        } else {
          type = s->field[i].layout.datatype;
          h5status = H5Tinsert(h5datatype, s->field[i].layout.name,
              s->field[i].layout.field_offset, type);
          H5CheckStatus(h5status);
        }
      }
    }
  }
  return h5datatype;
}

/**
 * @brief
 * Commit the compound datatype for attribute to the file
 *
 * @param s The attribute structure
 *
 * @return The committed datatype on success, CORE_ERR_HDF on failure
 */
hid_t CommitAttrFileDatatype(attr *s) {
  hid_t h5datatype, type;
  hsize_t adims[MAX_RANK];
  herr_t h5status;
  unsigned int i = 0, j = 0;
  size_t c_size = 0;

  h5datatype = H5Tcreate(H5T_COMPOUND, s->layout.compound_size);
  for (i = 0; i < s->compound_fields; i++) {
    if (s->field[i].layout.datatype > 0) {

      // Special handling for char datatype
      if (s->field[i].layout.datatype == H5T_NATIVE_CHAR ||
          s->field[i].layout.datatype == H5T_NATIVE_UCHAR) {
        type = H5Tcopy(H5T_C_S1);
        c_size = s->field[i].layout.elements;
        H5Tset_size(type, c_size);
        h5status = H5Tinsert(h5datatype, s->field[i].layout.name,
            s->field[i].layout.field_offset, type);
        H5CheckStatus(h5status);
        H5Tclose(type);

      } else {
        if (s->field[i].layout.elements > 1) {
          for (j = 0; j < MAX_RANK; j++) {
            adims[j] = s->field[i].layout.dims[j];
          }
          type = H5Tarray_create(s->field[i].layout.datatype, s->field[i].layout.rank, adims);
          h5status = H5Tinsert(h5datatype, s->field[i].layout.name,
              s->field[i].layout.field_offset, type);
          H5CheckStatus(h5status);
          H5Tclose(type);
        } else {
          type = s->field[i].layout.datatype;
          h5status = H5Tinsert(h5datatype, s->field[i].layout.name,
              s->field[i].layout.field_offset, type);
          H5CheckStatus(h5status);
        }
      }
    }
  }
  return h5datatype;
}

/**
 * @brief Get the proper padding for a datatype
 *
 * @param elements The number of data elements
 * @param datatype_size The datatype size of the data element
 *
 * @return Calculated padding
 */
size_t GetPadding(unsigned int elements, size_t datatype_size) {
  size_t size, reminder = 0, alignment = 0;

  alignment = sizeof(double);
  size = elements * datatype_size;

  if (size > alignment) {
    reminder = size % alignment;
    if (reminder > 0) return alignment - reminder;
  }

  if (size < alignment) {
    return alignment - size;
  }

  if (size == alignment) return 0;
  return 0;
}

