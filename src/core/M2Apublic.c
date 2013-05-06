/**
 * @file
 * Common functions (public API)
 */
#include "M2Apublic.h"

/**
 * @brief Creates name string
 *
 * @param prefix The prefix for the name
 * @param name The name string
 * @param suffix The suffix for the name
 * @param extension The extension for the name
 *
 * @return The allocated char array with the name built with prefix, name, suffix and
 * extension values. It must be freed after usage.
 */
char* Name(char *prefix, char *name, char *suffix, char *extension) {
  char *fname;
  size_t preflen, nlen, sufflen, extlen, filelen;

  preflen = strlen(prefix);
  nlen = strlen(name);
  sufflen = strlen(suffix);
  extlen = strlen(extension);

  filelen = preflen + nlen + sufflen + extlen + 1;

  fname = calloc(filelen, sizeof(char*));
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
int Copy(char *in, char *out) {
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

