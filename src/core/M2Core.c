/**
 * @file
 * Core-related functions
 */
#include "M2Core.h"

/**
 * @brief The Welcome message
 */
void Welcome() {
  Message(MESSAGE_INFO, "This is Mechanic, v.%s.%s.%s\n",
      PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR, PACKAGE_VERSION_PATCH);
  Message(MESSAGE_INFO, "Author: %s\n", PACKAGE_AUTHOR);
  Message(MESSAGE_INFO, "Bugs/features/support: %s\n", PACKAGE_BUGREPORT);
  Message(MESSAGE_INFO, "%s\n", PACKAGE_URL);
  Message(MESSAGE_OUTPUT, "\n");
}

/**
 * @brief Creates the valid Mechanic file header
 *
 * @param m The module pointer
 * @param h5location The HDF5 location (should be the master file)
 *
 * @return 0 on success, error code otherwise
 */
int MechanicHeader(module *m, hid_t h5location) {
  double api;
  hid_t attr_s, attr_d, ctype, memtype;
  herr_t h5status;
  hsize_t sdims[1] = {1};
  char *module_name;

  // The API version
  api = PACKAGE_VERSION_API;

  attr_s = H5Screate(H5S_SCALAR);
  attr_d = H5Acreate2(h5location, "API", H5T_NATIVE_DOUBLE, attr_s, H5P_DEFAULT, H5P_DEFAULT);
  H5Awrite(attr_d, H5T_NATIVE_DOUBLE, &api);
  H5Sclose(attr_s);
  H5Aclose(attr_d);

  // The module name
  module_name = Option2String("core", "module", m->layer.setup.head);

  ctype = H5Tcopy(H5T_C_S1);
  h5status = H5Tset_size(ctype, CONFIG_LEN);
  H5CheckStatus(h5status);
  
  memtype = H5Tcopy(H5T_C_S1);
  h5status = H5Tset_size(memtype, CONFIG_LEN);
  H5CheckStatus(h5status);

  attr_s = H5Screate_simple(1, sdims, NULL);
      
  attr_d = H5Acreate2(h5location, "MODULE", memtype, attr_s, H5P_DEFAULT, H5P_DEFAULT);
  H5CheckStatus(attr_d);
  H5Awrite(attr_d, memtype, module_name);
      
  H5Aclose(attr_d);
  H5Sclose(attr_s);
  H5Tclose(ctype);
  H5Tclose(memtype);

  return SUCCESS;
}

