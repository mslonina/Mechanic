/**
 * @file
 * The Master node (MPI Blocking communication)
 */
#include "Taskfarm.h"

/**
 * @brief Performs master node operations
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int MasterBlocking(module *m, pool *p) {
  int mstat = SUCCESS;
  int i = 0, k = 0, cid = 0, terminated_nodes = 0;
  int tag;
  int header[HEADER_SIZE] = HEADER_INIT;
  int c_offset = 0;
  int **board_buffer = NULL;
  int completed = 0, send_node;
  int x, y;

  MPI_Status mpi_status;
  
  storage *send_buffer, *recv_buffer;

  task *t = NULL;
  checkpoint *c = NULL;

  /* Initialize the temporary task board buffer */
  board_buffer = AllocateInt2D(p->board->layout.rank, p->board->layout.dim);
  if (m->mode != RESTART_MODE) {
    memset(board_buffer[0], TASK_AVAILABLE, p->pool_size*sizeof(int));
  } else {
    ReadData(p->board, board_buffer[0]);

    /* Prepare the task board */
    for (x = 0; x < p->board->layout.dim[0]; x++) {
      for (y = 0; y < p->board->layout.dim[1]; y++) {
        if (board_buffer[x][y] == TASK_IN_USE) {
          board_buffer[x][y] = TASK_TO_BE_RESTARTED;
        }
        if (board_buffer[x][y] == TASK_FINISHED) {
          completed++;
        }
      }
    }
  }

  /* Data buffers */
  send_buffer = calloc(sizeof(storage), sizeof(storage));
  if (!send_buffer) Error(CORE_ERR_MEM);

  recv_buffer = calloc(sizeof(storage), sizeof(storage));
  if (!recv_buffer) Error(CORE_ERR_MEM);

  /* Initialize the task and checkpoint */
  t = TaskLoad(m, p, 0);
  c = CheckpointLoad(m, p, 0);

  /* Initialize data buffers */
  send_buffer->layout.size = sizeof(int) * (HEADER_SIZE);
  for (k = 0; k < m->task_banks; k++) {
    send_buffer->layout.size +=
      GetSize(p->task->storage[k].layout.rank, p->task->storage[k].layout.dim)*p->task->storage[k].layout.datatype_size;
  }
  mstat = Allocate(send_buffer, send_buffer->layout.size, sizeof(char));

  recv_buffer->layout.size = send_buffer->layout.size;
  mstat = Allocate(recv_buffer, recv_buffer->layout.size, sizeof(char));

  /* Send initial tasks to all workers */
  for (i = 1; i < m->mpi_size; i++) {
    mstat = GetNewTask(m, p, t, board_buffer);
    CheckStatus(mstat);

    if (mstat != NO_MORE_TASKS) {
      mstat = Pack(m, send_buffer->memory, p, t, TAG_DATA);
      CheckStatus(mstat);
      board_buffer[t->location[0]][t->location[1]] = TASK_IN_USE;
    } else {
      tag = TAG_TERMINATE;
      memcpy(send_buffer->memory, &tag, sizeof(int));
      terminated_nodes++;
    }

    MPI_Send(&(send_buffer->memory[0]), send_buffer->layout.size, MPI_CHAR,
        i, TAG_DATA, MPI_COMM_WORLD);
  }

  /* The task farm loop (Blocking communication) */
  while (1) {

    /* Flush checkpoint buffer and write data, reset counter */
    if (c->counter > (c->size-1)) {

      WriteData(p->board, board_buffer[0]);
      mstat = CheckpointPrepare(m, p, c);
      CheckStatus(mstat);

      mstat = CheckpointProcess(m, p, c);
      CheckStatus(mstat);

      cid++;

      /* Reset the checkpoint */
      CheckpointReset(m, p, c, cid);
    }

    /* Wait for any operation to complete */
    MPI_Recv(&(recv_buffer->memory[0]), recv_buffer->layout.size, MPI_CHAR,
      MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);

      send_node = mpi_status.MPI_SOURCE;

    /* Get the data header */
    memcpy(header, recv_buffer->memory, sizeof(int) * (HEADER_SIZE));

    if (header[0] == TAG_RESULT) {
      c_offset = c->counter*(int)recv_buffer->layout.size;
      memcpy(c->storage->memory + c_offset, recv_buffer->memory, recv_buffer->layout.size);
    }

    board_buffer[header[3]][header[4]] = header[2];

    c->counter++;
    completed++;

    mstat = GetNewTask(m, p, t, board_buffer);
    CheckStatus(mstat);

    if (mstat != NO_MORE_TASKS) {

      mstat = Pack(m, send_buffer->memory, p, t, TAG_DATA);
      CheckStatus(mstat);

      board_buffer[t->location[0]][t->location[1]] = TASK_IN_USE;

      MPI_Send(&(send_buffer->memory[0]), send_buffer->layout.size, MPI_CHAR,
          send_node, TAG_DATA, MPI_COMM_WORLD);

    }

    if (completed == p->pool_size) break;
  }

  WriteData(p->board, board_buffer[0]);
  mstat = CheckpointPrepare(m, p, c);
  CheckStatus(mstat);

  mstat = CheckpointProcess(m, p, c);
  CheckStatus(mstat);

  /* Terminate all workers */
  for (i = 1; i < m->mpi_size - terminated_nodes; i++) {
    tag = TAG_TERMINATE;
    memcpy(send_buffer->memory, &tag, sizeof(int));

    MPI_Send(&(send_buffer->memory[0]), send_buffer->layout.size, MPI_CHAR,
        i, TAG_DATA, MPI_COMM_WORLD);

  }

  /* Finalize */
  CheckpointFinalize(m, p, c);
  TaskFinalize(m, p, t);

  if (send_buffer) {
    if (send_buffer->memory) Free(send_buffer);
    free(send_buffer);
  }

  if (recv_buffer) {
    if (recv_buffer->memory) Free(recv_buffer);
    free(recv_buffer);
  }

  if (board_buffer) {
    FreeIntBuffer(board_buffer);
  }

  return mstat;
}

