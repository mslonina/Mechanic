/**
 * @file
 * The Task Farm
 */
#include "Taskfarm.h"

/**
 * @function
 */
int Taskfarm(module *m) {
  int mstat = 0;

  if (m->node == MASTER) Master(m);
  if (m->node != MASTER) Worker(m);

  return mstat;
}
