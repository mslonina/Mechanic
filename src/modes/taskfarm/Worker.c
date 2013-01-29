/**
 * @file
 * The Worker node (MPI Blocking communication)
 */
#include "Taskfarm.h"

/**
 * @brief Performs worker node operations
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int Worker(module *m, pool *p) {
  int mstat = SUCCESS;
  int tag;
  int k = 0;

  MPI_Status recv_status;

  task *t = NULL;
  checkpoint *c = NULL;
  storage *send_buffer = NULL, *recv_buffer = NULL;

  /* Initialize the task and checkpoint */
  t = TaskLoad(m, p, 0);
  c = CheckpointLoad(m, p, 0);

  /* Data buffers */
  send_buffer = calloc(1, sizeof(storage));
  if (!send_buffer) Error(CORE_ERR_MEM);

  recv_buffer = calloc(1, sizeof(storage));
  if (!recv_buffer) Error(CORE_ERR_MEM);

  /* Initialize data buffers */
  send_buffer->layout.size = sizeof(int) * (HEADER_SIZE);
  for (k = 0; k < m->task_banks; k++) {
    send_buffer->layout.size +=
      GetSize(p->task->storage[k].layout.rank, p->task->storage[k].layout.dims)*p->task->storage[k].layout.datatype_size;
  }

  recv_buffer->layout.size = send_buffer->layout.size;
  
  send_buffer->memory = malloc(send_buffer->layout.size);
  if (!send_buffer->memory) Error(CORE_ERR_MEM);
  
  recv_buffer->memory = malloc(recv_buffer->layout.size);
  if (!recv_buffer->memory) Error(CORE_ERR_MEM);

  while (1) {

    MPI_Recv(&(recv_buffer->memory[0]), recv_buffer->layout.size, MPI_CHAR,
        MASTER, MPI_ANY_TAG, MPI_COMM_WORLD, &recv_status);

    mstat = Unpack(m, &(recv_buffer->memory[0]), p, t, &tag);
    CheckStatus(mstat);

    mstat = Receive(m->node, MASTER, tag, m, p, &(recv_buffer->memory[0]));
    CheckStatus(mstat);

    if (tag == TAG_TERMINATE) {
      break;
    } else {

      Message(MESSAGE_DEBUG, "Worker recv: %d %d %d %d\n", t->tid,
          t->location[0], t->location[1], t->location[2]);

      mstat = TaskPrepare(m, p, t);
      CheckStatus(mstat);

      mstat = TaskProcess(m, p, t);
      CheckStatus(mstat);

      if (mstat == TASK_CHECKPOINT) {
        t->status = TASK_IN_USE;
        tag = TAG_CHECKPOINT;
        t->cid++;
      }

      if (mstat == TASK_FINALIZE) {
        t->status = TASK_FINISHED;
        tag = TAG_RESULT;
      }

      mstat = Pack(m, send_buffer->memory, p, t, tag);
      CheckStatus(mstat);

      MPI_Send(&(send_buffer->memory[0]), send_buffer->layout.size, MPI_CHAR,
          MASTER, TAG_DATA, MPI_COMM_WORLD);

      mstat = Send(m->node, MASTER, TAG_DATA, m, p);
      CheckStatus(mstat);

      if (t->status == TASK_FINISHED) {
        TaskReset(m, p, t, 0);
      }
    }
  }

  /* Finalize */
  CheckpointFinalize(m, p, c);
  TaskFinalize(m, p, t);

  if (send_buffer) {
    free(send_buffer->memory);
    free(send_buffer);
  }

  if (recv_buffer) {
    free(recv_buffer->memory);
    free(recv_buffer);
  }

  return mstat;
}

