/**
 * @file
 * The Task Farm Mode
 */
#include "mechanic.h"
#include "mechanic_internals.h"
#include "mechanic_mode_farm.h"

int mechanic_mode_farm(mechanic_internals *handler) {

  int mstat;

  if (handler->node == MECHANIC_MPI_MASTER_NODE) {
    mstat = mechanic_mode_farm_master(handler);
    mechanic_check_mstat(mstat);
  } else {
    mstat = mechanic_mode_farm_worker(handler);
    mechanic_check_mstat(mstat);
  }

  return mstat;
}

