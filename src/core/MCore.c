/**
 * @file
 * The core-related functions
 */
#include "MCore.h"

/**
 * @brief The Welcome message
 */
void Welcome() {
  Message(MESSAGE_INFO, "This is Mechanic2\n");
}

/**
 * @brief The Prepare hook
 *
 * @param m The module pointer
 *
 * @return 0 on success, error code otherwise
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
 * @brief The Process hook
 *
 * @param m The module pointer
 *
 * @return 0 on success, error code otherwise
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

