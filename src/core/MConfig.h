#ifndef MECHANIC_CONFIG_H
#define MECHANIC_CONFIG_H

#include "Mechanic2.h"

/**
 * @var typedef struct ccd_t
 * @brief Helper datatype used for HDF5 storage
 *
 * @param name
 *  Name of the variable
 * 
 * @param value
 *  Value of the variable
 *
 * @param type
 *  Type of the variable
 */
typedef struct{
  char name[LRC_CONFIG_LEN];
  char value[LRC_CONFIG_LEN];
  int type;
} ccd_t;

#endif
