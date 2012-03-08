#ifndef MECHANIC_TYPES_H
#define MECHANIC_TYPES_H

/**
 * @class
 * Defines the layer
 *
 * @param void*
 *  Pointer to the dlopened() shared library
 * @param init
 *  Initialization structure
 * @param setup
 *  Setup structure
 */
typedef struct {
  void *handler;
  init init;
  setup setup;
} layer; 

/**
 * @class
 * Defines the module
 *
 * @param layer
 *  The actual module layer
 * @param fallback
 *  The fallback module layer
 */
typedef struct {
  char filename[LRC_CONFIG_LEN];
  hid_t datafile;
  hid_t location;
  int node;
  int mpi_size;
  layer layer;
  layer fallback;
} module;

/**
 * @class
 * Basic dynamic query type
 */
typedef int (query) ();

#endif
