/**
 * @file
 * Common functions and defines
 */
#include "MCommon.h"

/**
 * @brief Creates name
 *
 * @param prefix The prefix for the name
 * @param name The name
 * @param suffix The suffix for the name
 * @param extension The extension for the name
 *
 * @return The allocated char array with the name built with prefix, name, suffix and
 * extension values. It must be freed after usage.
 */
char* Name(char *prefix, char* name, char *suffix, char *extension) {
  char* fname;
  size_t preflen, nlen, sufflen, extlen, filelen;

  preflen = strlen(prefix);
  nlen = strlen(name);
  sufflen = strlen(suffix);
  extlen = strlen(extension);

  filelen = preflen + nlen + sufflen + extlen + 1;

  fname = calloc(filelen, sizeof(char));
  if (!fname) Error(CORE_ERR_MEM);

  strncpy(fname, prefix, preflen);
  fname[preflen] = CONFIG_NULL;

  strncat(fname, name, nlen);
  fname[preflen+nlen] = CONFIG_NULL;

  strncat(fname, suffix, sufflen);
  fname[preflen+nlen+sufflen] = CONFIG_NULL;

  strncat(fname, extension, extlen);
  fname[filelen] = CONFIG_NULL;

  return fname;
}

/**
 * @brief Copy files
 *
 * @param in The input filename
 * @param out The output filename
 *
 * @return 0 on success, error code otherwise
 */
int Copy(char* in, char* out) {
  int input;
  int output;
  int mstat = 0;
  char *c;
  struct stat st;

  /* Read and write files in binary mode */
  input = open(in, O_RDONLY);
  if (input < 0) {
    Message(MESSAGE_ERR, "Could not open input file %s\n", in);
    return CORE_ERR_HDF;
  }

  stat(in, &st);

  output = open(out, O_RDWR | O_CREAT | O_TRUNC, 0644);
  if (output < 0) {
    Message(MESSAGE_ERR, "Could not open output file %s\n", out);
    return CORE_ERR_HDF;
  }

  /* CHAR_BIT in limits.h */
  c = malloc(st.st_size * CHAR_BIT + 1);
  if (!c) return CORE_ERR_MEM;

  /* Read and write the whole file, without loops */
  mstat = read(input, c, st.st_size);
  if (mstat < 0) {
    Message(MESSAGE_ERR, "Could not read input file %s\n", in);
    return CORE_ERR_HDF;
  }

  mstat = write(output, c, st.st_size);
  if (mstat < 0) {
    Message(MESSAGE_ERR, "Could not write output file %s\n", out);
    return CORE_ERR_HDF;
  }

  free(c);

  if (close(input) < 0) {
    Message(MESSAGE_ERR, "Error closing input file\n");
    return CORE_ERR_HDF;
  }

  if (close(output) < 0) {
    Message(MESSAGE_ERR, "Error closing output file\n");
    return CORE_ERR_HDF;
  }

  return SUCCESS;
}

/**
 * @brief Creates the valid Mechanic file header
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
  attr_d = H5Acreate(h5location, "API", H5T_NATIVE_DOUBLE, attr_s, H5P_DEFAULT, H5P_DEFAULT);
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
      
  attr_d = H5Acreate(h5location, "MODULE", memtype, attr_s, H5P_DEFAULT, H5P_DEFAULT);
  H5CheckStatus(attr_d);
  H5Awrite(attr_d, memtype, module_name);
      
  H5Aclose(attr_d);
  H5Sclose(attr_s);
  H5Tclose(ctype);
  H5Tclose(memtype);

  return SUCCESS;
}

/**
 * @brief Validate the checkpoint file
 *
 * @param m The module pointer
 * @param filename The name of the file to be validated
 *
 * @return 0 on success, error code otherwise
 */
int Validate(module *m, char *filename) {
  double api;
  char module_name[CONFIG_LEN];
  hid_t h5location, attr, atype;
  herr_t h5status;

  h5location = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);

  // Get the API
  attr = H5Aopen_name(h5location, "API");
  h5status = H5Aread(attr, H5T_NATIVE_DOUBLE, &api);
  H5CheckStatus(h5status);
  H5Aclose(attr);

  // Get the module
  attr = H5Aopen_name(h5location, "MODULE");
  atype  = H5Aget_type(attr);
  h5status = H5Aread(attr, atype, &module_name);
  H5CheckStatus(h5status);
  H5Aclose(attr);

  H5Fclose(h5location);

  Message(MESSAGE_INFO, "Restart mode validation: API = %.2f, MODULE = %s\n", api, module_name);

  // Validate the module
  if (strcmp(module_name, Option2String("core", "module", m->layer.setup.head)) != 0) {
    Message(MESSAGE_ERR, "Module mismatch. Expected '%s', got '%s'\n", module_name, Option2String("core", "module", m->layer.setup.head)); 
    return -CORE_ERR_RESTART;
  }

  // Validate the API
  if (api < PACKAGE_VERSION_API) {
    Message(MESSAGE_ERR, "API mismatch. Expected %.2f, got %.2f\n", PACKAGE_VERSION_API, api);
    return -CORE_ERR_RESTART;
  }

  return SUCCESS;
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


