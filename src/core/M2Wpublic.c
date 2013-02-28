/**
 * @file
 * The work loop hooks (public API)
 */
#include "M2Wpublic.h"

/**
 * @brief The Prepare hook
 *
 * @param m The module pointer
 *
 * @return 0 on success, error code otherwise
 */
int M2Prepare(module *m) {
  int mstat = SUCCESS;
  void *s = NULL;
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
 * @param p The pointer to all pools
 *
 * @return 0 on success, error code otherwise
 */
int M2Process(module *m, pool **p) {
  int mstat = SUCCESS;
  void *s = NULL;
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
int M2NodePrepare(module *m, pool **all, pool *current) {
  int mstat = SUCCESS;
  void *s = NULL;
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
int M2NodeProcess(module *m, pool **all, pool *current) {
  int mstat = SUCCESS;
  void *s = NULL;
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
int M2LoopPrepare(module *m, pool **all, pool *current) {
  int mstat = SUCCESS;
  void *s = NULL;
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
int M2LoopProcess(module *m, pool **all, pool *current) {
  int mstat = SUCCESS;
  void *s = NULL;
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
int M2Send(int node, int dest, int tag, module *m, pool *p) {
  int mstat = SUCCESS;
  void *s = NULL;
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
int M2Receive(int node, int sender, int tag, module *m, pool *p, void *buffer) {
  int mstat = SUCCESS;
  void *s = NULL;
  query *q;

  q = LoadSym(m, "Receive", LOAD_DEFAULT);
  if (q) mstat = q(m->mpi_size, m->node, sender, tag, p, s, buffer);
  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief Pack the task data into memory buffer 
 *
 * @param m The module pointer
 * @param buffer The output pack buffer
 * @param p The current pool pointer
 * @param t The input task pointer
 * @param tag The MPI message tag
 *
 * @return 0 on success, error code otherwise
 */
int Pack(module *m, void *buffer, pool *p, task *t, int tag) {
  int mstat = SUCCESS, i = 0;
  int header[HEADER_SIZE] = HEADER_INIT;
  size_t position = 0, size = 0, header_size = 0;

  header[0] = tag;
  header[1] = t->tid;
  header[2] = t->status;
  header[3] = t->location[0];
  header[4] = t->location[1];
  header[5] = t->location[2];
  header[6] = t->cid;

  header_size = sizeof(int) * (HEADER_SIZE);
  position = header_size;
  mstat = CopyData(header, buffer, header_size);
  CheckStatus(mstat);

  if (tag != TAG_TERMINATE) {

    /* Task data */
    for (i = 0; i < p->task_banks; i++) {
      size = GetSize(t->storage[i].layout.rank, t->storage[i].layout.dims) * t->storage[i].layout.datatype_size;
      if (t->storage[i].layout.sync) {
        Message(MESSAGE_DEBUG, "[%s:%d] Packed dataset %s of rank %d = %zu bytes\n", __FILE__, __LINE__,
            t->storage[i].layout.name, t->storage[i].layout.rank, size);
        mstat = CopyData(t->storage[i].memory, buffer + position, size);
        CheckStatus(mstat);
      }
      position = position + size;
    }

  }

  return mstat;
}

/**
 * @brief Unpack the memory buffer into task structure
 *
 * @param m The module pointer
 * @param buffer The input pack buffer
 * @param p The current pool pointer
 * @param t The output task pointer
 * @param tag The MPI message tag
 *
 * @return 0 on success, error code otherwise
 */
int Unpack(module *m, void *buffer, pool *p, task *t, int *tag) {
  int mstat = SUCCESS, i = 0;
  int header[HEADER_SIZE] = HEADER_INIT;
  size_t position = 0, size = 0, header_size = 0;

  header_size = sizeof(int) * (HEADER_SIZE);
  position = header_size;
  mstat = CopyData(buffer, header, header_size);
  CheckStatus(mstat);

  *tag = header[0];
  t->tid = header[1];
  t->status = header[2];
  t->location[0]= header[3];
  t->location[1] = header[4];
  t->location[2] = header[5];
  t->cid = header[6];

  if (*tag != TAG_TERMINATE) {

    /* Task data */
    for (i = 0; i < p->task_banks; i++) {
      size = GetSize(t->storage[i].layout.rank, t->storage[i].layout.dims) * t->storage[i].layout.datatype_size;
      if (t->storage[i].layout.sync) {
        mstat = CopyData(buffer + position, t->storage[i].memory, size);
        CheckStatus(mstat);
      }
      position = position + size;
    }

  }

  return mstat;
}

