/**
 * @file
 * The core-related functions.
 */
#include "MCore.h"

/**
 * @function
 * The Welcome message
 */
void Welcome() {
  Message(MESSAGE_INFO, "This is Mechanic2.\n");
}

/**
 * @function
 * The Prepare hook
 */
int Prepare(module *m) {
  int mstat = 0;
  setup *s = &(m->layer.setup);
  query *q;

  if (m->node == MASTER) {
    q = LoadSym(m, "Prepare", LOAD_DEFAULT);
    if (q) mstat = q(m->filename, s);
  }

  return mstat;
}

/**
 * @function
 * The Process hook
 */
int Process(module *m) {
  int mstat = 0;
  setup *s = &(m->layer.setup);
  query *q;

  if (m->node == MASTER) {
    q = LoadSym(m, "Process", LOAD_DEFAULT);
    if (q) mstat = q(m->filename, s);
  }

  return mstat;
}
