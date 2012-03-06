/**
 * @file
 */
#include "MStorage.h"

/** 
 * Commits the storage layout to the HDF5 datafile
 */ 
int CommitStorageLayout(hid_t location, storage *s) {
  int mstat = 0, i = 0;
  hid_t dataspace, data;
  herr_t hdf_status;

  while (s[i].path) {
    if (s[i].use_hdf) {
      dataspace = H5Screate(s[i].type);
      if (s[i].type == H5S_SIMPLE) {
        hdf_status = H5Sset_extent_simple(dataspace, s[i].rank, s[i].dimsf, NULL);
      }
      data = H5Dcreate(location, s[i].path, s[i].datatype, dataspace, 
          H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

      H5Dclose(data);
      H5Sclose(dataspace);
    }
    i++;
  }

  if (hdf_status < 0) mstat = CORE_ERR_HDF;
  return mstat;
}
