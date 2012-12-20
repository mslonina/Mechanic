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
int Master(module *m, pool *p) {
  int mstat = SUCCESS, ice = 0;
  int i = 0, k = 0, cid = 0, terminated_nodes = 0;
  int tag;
  int header[HEADER_SIZE] = HEADER_INIT;
  int c_offset = 0;
  short ****board_buffer = NULL;
  int completed = 0, send_node;
  int x, y, z;

  MPI_Status mpi_status;
  
  storage *send_buffer = NULL, *recv_buffer = NULL;

  task *t = NULL;
  checkpoint *c = NULL;

  /* Initialize the temporary task board buffer */
  board_buffer = AllocateShort4(p->board);
  if (m->mode != RESTART_MODE) {
    memset(&board_buffer[0][0][0][0], TASK_AVAILABLE, p->board->layout.storage_elements * sizeof(short));
    WriteData(p->board, &board_buffer[0][0][0][0]);
  } else {
    ReadData(p->board, &board_buffer[0][0][0][0]);

    /* Prepare the task board */
    for (x = 0; x < p->board->layout.dims[0]; x++) {
      for (y = 0; y < p->board->layout.dims[1]; y++) {
        for (z = 0; z < p->board->layout.dims[2]; z++) {
          if (board_buffer[x][y][z][0] == TASK_IN_USE) {
            board_buffer[x][y][z][0] = TASK_TO_BE_RESTARTED;
          }
          if (board_buffer[x][y][z][0] == TASK_FINISHED) {
            completed++;
          }
        }
      }
    }
  }

  /* Data buffers */
  send_buffer = calloc(1, sizeof(storage));
  if (!send_buffer) Error(CORE_ERR_MEM);

  recv_buffer = calloc(1, sizeof(storage));
  if (!recv_buffer) Error(CORE_ERR_MEM);

  /* Initialize the task and checkpoint */
  t = TaskLoad(m, p, 0);
  c = CheckpointLoad(m, p, 0);

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

  /* Send initial tasks to all workers */
  for (i = 1; i < m->mpi_size; i++) {
    mstat = GetNewTask(m, p, t, board_buffer);
    CheckStatus(mstat);
    t->node = i;

    if (mstat != NO_MORE_TASKS) {
      mstat = Pack(m, send_buffer->memory, p, t, TAG_DATA);
      CheckStatus(mstat);
      board_buffer[t->location[0]][t->location[1]][t->location[2]][0] = TASK_IN_USE;
      board_buffer[t->location[0]][t->location[1]][t->location[2]][1] = t->node;
    } else {
      tag = TAG_TERMINATE;
      mstat = CopyData(&tag, send_buffer->memory, sizeof(int));
      CheckStatus(mstat);
      terminated_nodes++;
    }

    MPI_Send(&(send_buffer->memory[0]), (int)send_buffer->layout.size, MPI_CHAR,
        i, TAG_DATA, MPI_COMM_WORLD);
  }

  /* The task farm loop (Blocking communication) */
  while (1) {

    /* Check for ICE file */
    ice = Ice();
    if (ice == CORE_ICE) {
      Message(MESSAGE_WARN, "The ICE file has been detected. Flushing checkpoints\n");
    }

    /* Flush checkpoint buffer and write data, reset counter */
    if ((c->counter > (c->size-1)) || ice == CORE_ICE) {

      WriteData(p->board, &board_buffer[0][0][0][0]);
      mstat = CheckpointPrepare(m, p, c);
      CheckStatus(mstat);

      mstat = CheckpointProcess(m, p, c);
      CheckStatus(mstat);

      cid++;

      /* Reset the checkpoint */
      CheckpointReset(m, p, c, cid);
    }

    /* Do simple Abort on ICE */
    if (ice == CORE_ICE) Abort(CORE_ICE);

    /* Wait for any operation to complete */
    MPI_Recv(&(recv_buffer->memory[0]), (int)recv_buffer->layout.size, MPI_CHAR,
      MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &mpi_status);

    send_node = mpi_status.MPI_SOURCE;

    /* Get the data header */
    mstat = CopyData(recv_buffer->memory, header, sizeof(int) * (HEADER_SIZE));
    CheckStatus(mstat);

    if (header[0] == TAG_RESULT) {
      c_offset = c->counter * (int)recv_buffer->layout.size;
      mstat = CopyData(recv_buffer->memory, c->storage->memory + c_offset, recv_buffer->layout.size);

      board_buffer[header[3]][header[4]][header[5]][0] = header[2];
      board_buffer[header[3]][header[4]][header[5]][1] = send_node;

      c->counter++;
      completed++;

      mstat = GetNewTask(m, p, t, board_buffer);
      CheckStatus(mstat);

      if (mstat != NO_MORE_TASKS) {

        mstat = Pack(m, send_buffer->memory, p, t, TAG_DATA);
        CheckStatus(mstat);

        board_buffer[t->location[0]][t->location[1]][t->location[2]][0] = TASK_IN_USE;
        board_buffer[t->location[0]][t->location[1]][t->location[2]][1] = send_node;

        MPI_Send(&(send_buffer->memory[0]), (int)send_buffer->layout.size, MPI_CHAR,
            send_node, TAG_DATA, MPI_COMM_WORLD);

      } else {
        Message(MESSAGE_DEBUG, "Master: no more tasks after %d of %d completed\n", completed, p->pool_size);
      }
    }

    if (completed == p->pool_size) break;
  }

  Message(MESSAGE_DEBUG, "Completed %d tasks\n", completed);

  WriteData(p->board, &board_buffer[0][0][0][0]);
  mstat = CheckpointPrepare(m, p, c);
  CheckStatus(mstat);

  mstat = CheckpointProcess(m, p, c);
  CheckStatus(mstat);

  /* Terminate all workers */
  for (i = 1; i < m->mpi_size - terminated_nodes; i++) {
    tag = TAG_TERMINATE;
    mstat = CopyData(&tag, send_buffer->memory, sizeof(int));
    CheckStatus(mstat);

    MPI_Send(&(send_buffer->memory[0]), (int)send_buffer->layout.size, MPI_CHAR,
        i, TAG_DATA, MPI_COMM_WORLD);

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

  if (board_buffer) {
    free(board_buffer);
  }

  return mstat;
}

