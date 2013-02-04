/**
 * @file
 * Internal datatypes and function prototypes
 */
#ifndef MECHANIC_TYPES_H
#define MECHANIC_TYPES_H

/**
 * @struct layer
 * Defines the layer
 */
typedef struct {
  void *handler; /**< The module handler */
  init init; /**< The init structure */
  setup setup; /**< The setup structure */
} layer;

/**
 * @struct module
 * Defines the module
 */
typedef struct {
  char *filename; /**< The filename */
  hid_t datafile; /**< The datafile HDF5 pointer */
  hid_t h5location; /**< The HDF5 location pointer */
  int node; /**< The node ID */
  int mpi_size; /**< The MPI_COMM_WORLD size */
  int mode; /**< The running mode */
  int communication_type; /**< MPI communication type */
  layer layer; /**< The layer pointer */
  layer fallback; /**< The fallback layer pointer */
} module;

/**
 * @typedef query
 * Basic dynamic query type
 */
typedef int (query) ();

MPI_Datatype GetMpiDatatype(hid_t h5type);
hid_t GetHDF5Datatype(int ctype);

#endif
