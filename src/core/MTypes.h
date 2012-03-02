#ifndef MECHANIC_TYPES_H
#define MECHANIC_TYPES_H

#include "MMechanic2.h"
#include "MIncludes.h"

typedef struct {
  void *handler;
  setup setup;
} layer; 

typedef int (query) ();

#define MASTER 0

#endif
