/**
 * @file
 * The core-related functions
 */
#include "MCore.h"

/**
 * @brief The Welcome message
 */
void Welcome() {
  Message(MESSAGE_INFO, "This is Mechanic, v.%s.%s.%s\n",
      PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR, PACKAGE_VERSION_PATCH);
  Message(MESSAGE_INFO, "Author: %s\n", PACKAGE_AUTHOR);
  Message(MESSAGE_INFO, "Bugs/features/support: %s\n", PACKAGE_BUGREPORT);
  Message(MESSAGE_INFO, "%s\n", PACKAGE_URL);
  Message(MESSAGE_OUTPUT, "\n");
}

/**
 * @brief The Prepare hook
 *
 * @param m The module pointer
 *
 * @return 0 on success, error code otherwise
 */
int Prepare(module *m) {
  int mstat = SUCCESS;
  setup *s = &(m->layer.setup);
  query *q;

  q = LoadSym(m, "Prepare", LOAD_DEFAULT);
  if (q) mstat = q(m->node, m->filename, s);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief The Process hook
 *
 * @param m The module pointer
 *
 * @return 0 on success, error code otherwise
 */
int Process(module *m, pool **p) {
  int mstat = SUCCESS;
  setup *s = &(m->layer.setup);
  query *q;

  q = LoadSym(m, "Process", LOAD_DEFAULT);
  if (q) mstat = q(m->node, m->filename, p, s);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief The Node prepare hook
 *
 * @param m The module pointer
 * @param all The pointer to the all pools
 * @param current The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int NodePrepare(module *m, pool **all, pool *current) {
  int mstat = SUCCESS;
  setup *s = &(m->layer.setup);
  query *q;

  q = LoadSym(m, "NodePrepare", LOAD_DEFAULT);
  if (q) mstat = q(m->mpi_size, m->node, all, current, s);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief The Node process hook
 *
 * @param m The module pointer
 * @param all The pointer to the all pools
 * @param current The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int NodeProcess(module *m, pool **all, pool *current) {
  int mstat = SUCCESS;
  setup *s = &(m->layer.setup);
  query *q;

  q = LoadSym(m, "NodeProcess", LOAD_DEFAULT);
  if (q) mstat = q(m->mpi_size, m->node, all, current, s);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief The Loop prepare hook
 *
 * @param m The module pointer
 * @param all The pointer to the all pools
 * @param current The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int LoopPrepare(module *m, pool **all, pool *current) {
  int mstat = SUCCESS;
  setup *s = &(m->layer.setup);
  query *q;

  q = LoadSym(m, "LoopPrepare", LOAD_DEFAULT);
  if (q) mstat = q(m->mpi_size, m->node, all, current, s);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief The Loop process hook
 *
 * @param m The module pointer
 * @param all The pointer to the all pools
 * @param current The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int LoopProcess(module *m, pool **all, pool *current) {
  int mstat = SUCCESS;
  setup *s = &(m->layer.setup);
  query *q;

  q = LoadSym(m, "LoopProcess", LOAD_DEFAULT);
  if (q) mstat = q(m->mpi_size, m->node, all, current, s);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief The send hook
 *
 * @param node The current node
 * @param dest The destination node
 * @param tag The message tag
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int Send(int node, int dest, int tag, module *m, pool *p) {
  int mstat = SUCCESS;
  setup *s = &(m->layer.setup);
  query *q;

  q = LoadSym(m, "Send", LOAD_DEFAULT);
  if (q) mstat = q(m->mpi_size, m->node, dest, tag, p, s);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief The receive hook
 *
 * @param node The current node
 * @param sender The sender node
 * @param tag The message tag
 * @param m The module pointer
 * @param p The current pool pointer
 * @param buffer The raw data received
 *
 * @return 0 on success, error code otherwise
 */
int Receive(int node, int sender, int tag, module *m, pool *p, void *buffer) {
  int mstat = SUCCESS;
  setup *s = &(m->layer.setup);
  query *q;

  q = LoadSym(m, "Receive", LOAD_DEFAULT);
  if (q) mstat = q(m->mpi_size, m->node, sender, tag, p, s, buffer);
  CheckStatus(mstat);

  return mstat;
}

