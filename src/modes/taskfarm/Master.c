/**
 * @file
 * The Master node
 */
#include "Taskfarm.h"
mpi_message* MessageLoad(module *m, pool *p, size_t message_size);

mpi_message* MessageLoad(module *m, pool *p, size_t message_size) {
  mpi_message* message = NULL;

  message = calloc(1, sizeof(mpi_message));
  if (!message) Error(CORE_ERR_MEM);

  message->storage = calloc(1, sizeof(storage));
  if (!message->storage) Error(CORE_ERR_MEM);

  message->storage->layout = (schema) STORAGE_END;
  message->storage->memory = NULL;

  message->storage->memory = malloc(message_size);
  if (!message->storage->memory) Error(CORE_ERR_MEM);

  return message;
}

/**
 * @brief Performs master node operations
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int Master(module *m, pool *p) {
  int mstat = SUCCESS;
  int i = 0, k = 0, cid = 0, terminated_nodes = 0;
  int tag;
  int header[HEADER_SIZE] = HEADER_INIT;
  int c_offset = 0;
  int **board_buffer = NULL;
  int completed = 0;
  int *intags, index = 0, send_node;
  int x, y;
  size_t send_buffer_size = 0, recv_buffer_size = 0;
  char **send_buffer, **recv_buffer;

  mpi_message **sends, **recvs;

  MPI_Status *send_status, *recv_status, mpi_status;
  MPI_Request *send_request, *recv_request;

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

  /* Message buffers */
  intags = calloc(m->mpi_size, sizeof(uintptr_t));
  if (!intags) Error(CORE_ERR_MEM);

  send_status = calloc(m->mpi_size-1, sizeof(MPI_Status));
  if (!send_status) Error(CORE_ERR_MEM);

  send_request = calloc(m->mpi_size-1, sizeof(MPI_Request));
  if (!send_request) Error(CORE_ERR_MEM);

  recv_status = calloc(m->mpi_size-1, sizeof(MPI_Status));
  if (!recv_status) Error(CORE_ERR_MEM);

  recv_request = calloc(m->mpi_size-1, sizeof(MPI_Request));
  if (!recv_request) Error(CORE_ERR_MEM);

  /* Data buffers */
  sends = calloc(m->mpi_size-1, sizeof(mpi_message));
  recvs = calloc(m->mpi_size-1, sizeof(mpi_message));

  /* Initialize message tags */
  for (i = 0; i < m->mpi_size; i++) intags[i] = i;

  /* Initialize the task and checkpoint */
  t = TaskLoad(m, p, 0);
  c = CheckpointLoad(m, p, 0);

  /* Initialize data buffers */
  send_buffer_size = sizeof(int)*(HEADER_SIZE);
  for (k = 0; k < m->task_banks; k++) {
    send_buffer_size +=
      GetSize(p->task->storage[k].layout.rank, p->task->storage[k].layout.dim)*p->task->storage[k].layout.datatype_size;
  }

  recv_buffer_size = send_buffer_size;
  printf("buffer_size = %d\n", (int)send_buffer_size);

  for (i = 0; i < m->mpi_size-1; i++) {
    sends[i] = MessageLoad(m, p, send_buffer_size);
    recvs[i] = MessageLoad(m, p, recv_buffer_size);
  }

  send_buffer = malloc((m->mpi_size-1) * sizeof(char *));      // Allocate the row pointers
  send_buffer[0] = malloc(send_buffer_size * sizeof(char)); // Allocate all the elements
  for(i = 1; i < m->mpi_size-1; i++)
    send_buffer[i] = send_buffer[0] + i * (int)send_buffer_size;

  recv_buffer = malloc((m->mpi_size-1) * sizeof(char *));      // Allocate the row pointers
  recv_buffer[0] = malloc(recv_buffer_size * sizeof(char)); // Allocate all the elements
  for(i = 1; i < m->mpi_size-1; i++)
    recv_buffer[i] = recv_buffer[0] + i * (int)recv_buffer_size;

  /* Send initial tasks to all workers */
  for (i = 1; i < m->mpi_size; i++) {
    mstat = GetNewTask(m, p, t, board_buffer);
    CheckStatus(mstat);

    if (mstat != NO_MORE_TASKS) {
      mstat = Pack(m, &send_buffer[i-1], p, t, TAG_DATA);
      CheckStatus(mstat);
      board_buffer[t->location[0]][t->location[1]] = TASK_IN_USE;
    } else {
      tag = TAG_TERMINATE;
      mstat = CopyData(&tag, &send_buffer[i-1][0], sizeof(int));
      CheckStatus(mstat);
      terminated_nodes++;
    }

    MPI_Isend(&(send_buffer[i-1]), (int)send_buffer_size, MPI_CHAR,
        i, intags[i], MPI_COMM_WORLD, &send_request[i-1]);
    MPI_Irecv(&(recv_buffer[i-1]), (int)recv_buffer_size, MPI_CHAR,
        i, intags[i], MPI_COMM_WORLD, &recv_request[i-1]);
  }
  
  MPI_Waitall(m->mpi_size - 1, send_request, send_status);
//MPI_Barrier(MPI_COMM_WORLD);
  /* The task farm loop (Non-blocking communication) */
  while (1) {

    /* Flush checkpoint buffer and write data, reset counter */
    if (c->counter > (c->size - 1)) {
    
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
    MPI_Waitany(m->mpi_size-1, recv_request, &index, &mpi_status);

    if (index != MPI_UNDEFINED) {
      send_node = index+1;

      /* Get the data header */
      mstat = CopyData(&recv_buffer[index], header, sizeof(int) * (HEADER_SIZE));
      CheckStatus(mstat);
      
      Message(MESSAGE_OUTPUT, "RECV   header %2d %2d %2d location = %2d %2d\n",
          header[0], header[1], header[2], header[3], header[4]);
    
      if (header[0] == TAG_RESULT) {
      
        Message(MESSAGE_OUTPUT, "RESULT header %2d %2d %2d location = %2d %2d\n",
            header[0], header[1], header[2], header[3], header[4]);

        c_offset = c->counter * (int)recv_buffer_size;
        mstat = CopyData(&recv_buffer[index], c->storage->memory + c_offset, recv_buffer_size);
        CheckStatus(mstat);

        board_buffer[header[3]][header[4]] = header[2];
        
        c->counter++;
        completed++;

        mstat = GetNewTask(m, p, t, board_buffer);
        CheckStatus(mstat);

MPI_Barrier(MPI_COMM_WORLD);
        if (mstat != NO_MORE_TASKS) {

          mstat = Pack(m, &(send_buffer[index][0]), p, t, TAG_DATA);
          CheckStatus(mstat);
          board_buffer[t->location[0]][t->location[1]] = TASK_IN_USE;

          MPI_Isend(&(send_buffer[index]), (int)send_buffer_size, MPI_CHAR,
              send_node, intags[send_node], MPI_COMM_WORLD, &send_request[index]);

          MPI_Irecv(&(recv_buffer[index]), (int)recv_buffer_size, MPI_CHAR,
              send_node, intags[send_node], MPI_COMM_WORLD, &recv_request[index]);

          MPI_Wait(&send_request[index], &send_status[index]);
        }
      }
    }
  
    if (completed == p->pool_size) break;
  }
  
  MPI_Barrier(MPI_COMM_WORLD);
  WriteData(p->board, board_buffer[0]);
  mstat = CheckpointPrepare(m, p, c);
  CheckStatus(mstat);

  mstat = CheckpointProcess(m, p, c);
  CheckStatus(mstat);

  /* Terminate all workers */
  for (i = 1; i < m->mpi_size - terminated_nodes; i++) {
    tag = TAG_TERMINATE;
    mstat = CopyData(&tag, sends[i-1]->storage->memory, sizeof(int));
    CheckStatus(mstat);

    MPI_Isend(&(sends[i-1]->storage->memory), (int)send_buffer_size, MPI_CHAR,
        i, intags[i], MPI_COMM_WORLD, &send_request[i-1]);

  }

  MPI_Waitall(m->mpi_size - 1, send_request, send_status);

  //MPI_Barrier(MPI_COMM_WORLD);
  /* Finalize */
  CheckpointFinalize(m, p, c);
  TaskFinalize(m, p, t);

  if (intags) free(intags);
  if (send_status) free(send_status);
  if (send_request) free(send_request);
  if (recv_status) free(recv_status);
  if (recv_request) free(recv_request);

  //if (send_buffer) {
  //  for (i = 0; i < m->mpi_size; i++) {
  //    if (send_buffer[i].memory) Free(&send_buffer[i]);
  //  }
  //  free(send_buffer);
  //}

  //if (recv_buffer) {
  //  for (i = 0; i < m->mpi_size; i++) {
  //    if (recv_buffer[i].memory) Free(&recv_buffer[i]);
  //  }
  //  free(recv_buffer);
  //}

  if (board_buffer) {
    FreeIntBuffer(board_buffer);
  }

  for (i = 0; i < m->mpi_size - 1; i++) {
  //  free(sends[i]->storage->memory);
    free(sends[i]->storage);
    
  //  free(recvs[i]->storage->memory);
    free(recvs[i]->storage);
  }

  free(sends);
  free(recvs);

  return mstat;
}

