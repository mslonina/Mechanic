/**
 * @file
 * The datatypes related functions
 */
#include "M2Aprivate.h"

/**
 * @brief Performs Config to HDF5 datatype mapping
 * 
 * @param ctype The config type to map
 *
 * @return The proper HDF5 datatype
 */
hid_t GetHDF5Datatype(int ctype) {

  /**
   * CHAR
   */
  if (ctype == C_STRING) return H5T_NATIVE_CHAR;
  
  /**
   * INT
   */
  if (ctype == C_INT) return H5T_NATIVE_INT;
  
  /**
   * LONG INT
   */
  if (ctype == C_LONG) return H5T_NATIVE_LONG;
  
  /**
   * FLOAT
   */
  if (ctype == C_FLOAT) return H5T_NATIVE_FLOAT;
  
  /**
   * DOUBLE
   */
  if (ctype == C_DOUBLE) return H5T_NATIVE_DOUBLE;
  
  /**
   * Default
   */
  return H5T_NATIVE_INT;
}

/**
 * @brief Performs HDF5 to MPI datatype mapping
 *
 * @param h5type The HDF5 datatype to map
 *
 * @return The proper MPI datatype
 */
MPI_Datatype GetMpiDatatype(hid_t h5type) {

  /**
   * signed char
   * H5T_STD_I8BE or H5T_STD_I8LE
   */
  if (h5type == H5T_NATIVE_CHAR) return MPI_CHAR;

  /**
   * unsigned char
   * H5T_STD_U8BE or H5T_STD_U8LE
   */
  if (h5type == H5T_NATIVE_UCHAR) return MPI_UNSIGNED_CHAR;

  /**
   * signed int
   * H5T_STD_I32BE or H5T_STD_I32LE
   */
  if (h5type == H5T_NATIVE_INT) return MPI_INT;

  /**
   * signed short int
   * H5T_STD_I16BE or H5T_STD_I16LE
   */
  if (h5type == H5T_NATIVE_SHORT) return MPI_SHORT;

  /**
   * signed long int
   * H5T_STD_I32BE, H5T_STD_I32LE, H5T_STD_I64BE or H5T_STD_I64LE
   */
  if (h5type == H5T_NATIVE_LONG) return MPI_LONG;

  /**
   * signed long long int
   * H5T_STD_I64BE or H5T_STD_I64LE
   */
  if (h5type == H5T_NATIVE_LLONG) return MPI_LONG_LONG;

  /**
   * unsigned int
   * H5T_STD_U32BE or H5T_STD_U32LE
   */
  if (h5type == H5T_NATIVE_UINT) return MPI_UNSIGNED;

  /**
   * unsigned short int
   * H5T_STD_U16BE or H5T_STD_U16LE
   */
  if (h5type == H5T_NATIVE_USHORT) return MPI_UNSIGNED_SHORT;

  /**
   * unsigned long int
   * H5T_STD_U32BE, H5T_STD_U32LE, H5T_STD_U64BE or H5T_STD_U64LE
   */
  if (h5type == H5T_NATIVE_ULONG) return MPI_UNSIGNED_LONG;

  /**
   * unsigned long long int
   * H5T_STD_U64BE or H5T_STD_U64LE
   */
  if (h5type == H5T_NATIVE_ULLONG) return MPI_UNSIGNED_LONG_LONG;

  /**
   * float
   * H5T_IEEE_F32BE or H5T_IEEE_F32LE
   */
  if (h5type == H5T_NATIVE_FLOAT) return MPI_FLOAT;

  /**
   * double
   * H5T_IEEE_F64BE or H5T_IEEE_F64LE
   */
  if (h5type == H5T_NATIVE_DOUBLE) return MPI_DOUBLE;

  /**
   * Default fallback to MPI_CHAR
   */
  return MPI_CHAR;
}

