#include "Mechanic2.h"
#include "MTypes.h"

/**
 * @function
 * Performs datatypes mapping
 */
MPI_Datatype GetMpiDatatype(hid_t h5type) {
  
  // Character types
  if (h5type == H5T_NATIVE_CHAR) return MPI_CHAR;
  if (h5type == H5T_NATIVE_UCHAR) return MPI_UNSIGNED_CHAR;

  // Integer types
  if (h5type == H5T_NATIVE_INT) return MPI_INT;
  if (h5type == H5T_NATIVE_SHORT) return MPI_SHORT;
  if (h5type == H5T_NATIVE_LONG) return MPI_LONG;
  if (h5type == H5T_NATIVE_LLONG) return MPI_LONG_LONG;
  if (h5type == H5T_NATIVE_UINT) return MPI_UNSIGNED;
  if (h5type == H5T_NATIVE_USHORT) return MPI_UNSIGNED_SHORT;
  if (h5type == H5T_NATIVE_ULONG) return MPI_UNSIGNED_LONG;
  if (h5type == H5T_NATIVE_ULLONG) return MPI_UNSIGNED_LONG_LONG;
  // Float types
  if (h5type == H5T_NATIVE_FLOAT) return MPI_FLOAT;
  if (h5type == H5T_NATIVE_DOUBLE) return MPI_DOUBLE;

  return MPI_CHAR;
}
