/**
 * @file
 * The datatypes related functions
 */
#include "Mechanic2.h"
#include "MTypes.h"

/**
 * @brief Performs HDF5 to MPI datatype mapping
 *
 * All native HDF5 datatypes are supported:
 *
 * | C datatype             | MPI datatype           | HDF5 native datatype | HDF5 platform datatype           |
 * |:-----------------------|:-----------------------|:---------------------|:---------------------------------|
 * | signed char            | MPI_CHAR               | H5T_NATIVE_CHAR      | H5T_STD_I8BE or H5T_STD_I8LE     |
 * | unsigned char          | MPI_UNSIGNED_CHAR      | H5T_NATIVE_UCHAR     | H5T_STD_U8BE or H5T_STD_U8LE     |
 * | signed int             | MPI_INT                | H5T_NATIVE_INT       | H5T_STD_I32BE or H5T_STD_I32LE   |
 * | signed short int       | MPI_SHORT              | H5T_NATIVE_SHORT     | H5T_STD_I16BE or H5T_STD_I16LE   |
 * | signed long int        | MPI_LONG               | H5T_NATIVE_LONG      | H5T_STD_I32BE, H5T_STD_I32LE,    |
 * |                        |                        |                      | H5T_STD_I64BE or H5T_STD_I64LE   |
 * | signed long long int   | MPI_LONG_LONG          | H5T_NATIVE_LLONG     | H5T_STD_I64BE or H5T_STD_I64LE   |
 * | unsigned int           | MPI_UNSIGNED           | H5T_NATIVE_UINT      | H5T_STD_U32BE or H5T_STD_U32LE   |
 * | unsigned short int     | MPI_UNSIGNED_SHORT     | H5T_NATIVE_USHORT    | H5T_STD_U16BE or H5T_STD_U16LE   |
 * | unsigned long long int | MPI_UNSIGNED_LONG_LONG | H5T_NATIVE_ULLONG    | H5T_STD_U64BE or H5T_STD_U64LE   |
 * | float                  | MPI_FLOAT              | H5T_NATIVE_FLOAT     | H5T_IEEE_F32BE or H5T_IEEE_F32LE |
 * | double                 | MPI_DOUBLE             | H5T_NATIVE_DOUBLE    | H5T_IEEE_F64BE or H5T_IEEE_F64LE |
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

