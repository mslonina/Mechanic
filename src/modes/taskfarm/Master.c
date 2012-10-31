/**
 * @file
 * The Master node
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
  int mstat = SUCCESS;
  int i = 0, k = 0, cid = 0, terminated_nodes = 0;
  int tag;
  int header[HEADER_SIZE] = HEADER_INIT;
  int c_offset = 0;
  int **board_buffer = NULL;
  int completed = 0;
  int *intags, index = 0, req_flag, send_node;
  int x, y;
  int send_buffer_size = 0, recv_buffer_size = 0;

  storage *send_buffer, *recv_buffer;

  MPI_Status *send_status, *recv_status, mpi_status;
  MPI_Request *send_request, *recv_request;

  task *t = NULL;
  checkpoint *c = NULL;
  char *taddr = NULL;

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
  intags = calloc(m->mpi_size * sizeof(uintptr_t), sizeof(uintptr_t));
  if (!intags) Error(CORE_ERR_MEM);

  send_status = calloc((m->mpi_size-1) * sizeof(MPI_Status), sizeof(MPI_Status));
  if (!send_status) Error(CORE_ERR_MEM);

  send_request = calloc((m->mpi_size-1) * sizeof(MPI_Request), sizeof(MPI_Request));
  if (!send_request) Error(CORE_ERR_MEM);

  recv_status = calloc((m->mpi_size-1) * sizeof(MPI_Status), sizeof(MPI_Status));
  if (!recv_status) Error(CORE_ERR_MEM);

  recv_request = calloc((m->mpi_size-1) * sizeof(MPI_Request), sizeof(MPI_Request));
  if (!recv_request) Error(CORE_ERR_MEM);

  /* Data buffers */
  send_buffer = calloc(m->mpi_size*sizeof(storage), sizeof(storage));
  if (!send_buffer) Error(CORE_ERR_MEM);

  recv_buffer = calloc(m->mpi_size*sizeof(storage), sizeof(storage));
  if (!recv_buffer) Error(CORE_ERR_MEM);

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

  for (i = 0; i < m->mpi_size; i++) {
    mstat = Allocate(&send_buffer[i], (size_t)send_buffer_size, sizeof(char));
    mstat = Allocate(&recv_buffer[i], (size_t)recv_buffer_size, sizeof(char));
  }

  /* Send initial tasks to all workers */
  for (i = 1; i < m->mpi_size; i++) {
    mstat = GetNewTask(m, p, t, board_buffer);
    CheckStatus(mstat);

    if (mstat != NO_MORE_TASKS) {
      mstat = Pack(m, &send_buffer[i].memory, p, t, TAG_DATA);
      CheckStatus(mstat);
      board_buffer[t->location[0]][t->location[1]] = TASK_IN_USE;
    } else {
      tag = TAG_TERMINATE;
      memcpy(&send_buffer[i].memory, &tag, sizeof(int));
      terminated_nodes++;
    }

    MPI_Isend(&send_buffer[i].memory, send_buffer_size, MPI_CHAR,
        i, intags[i], MPI_COMM_WORLD, &send_request[i-1]);
    MPI_Irecv(&recv_buffer[i].memory, recv_buffer_size, MPI_CHAR,
        i, intags[i], MPI_COMM_WORLD, &recv_request[i-1]);
  }
  
  MPI_Waitall(m->mpi_size - 1, send_request, send_status);

  /* The task farm loop (Non-blocking communication) */
  while (1) {

    /* Test for any completed request */
    MPI_Testany(m->mpi_size-1, recv_request, &index, &req_flag, &mpi_status);

    if (!req_flag) {

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
      MPI_Waitany(m->mpi_size-1, recv_request, &index, &mpi_status);
      send_node = index+1;
    
      /* Get the data header */
      memcpy(header, &recv_buffer[send_node].memory, sizeof(int) * (HEADER_SIZE));
    
      if (header[0] == TAG_RESULT) {
        c_offset = c->counter*recv_buffer_size;
        memcpy(c->storage->memory + c_offset, &recv_buffer[send_node].memory, (size_t)recv_buffer_size);
      }

      board_buffer[header[3]][header[4]] = header[2];

      c->counter++;
      completed++;

      mstat = GetNewTask(m, p, t, board_buffer);
      CheckStatus(mstat);

      if (mstat != NO_MORE_TASKS) {

        mstat = Pack(m, &send_buffer[send_node].memory, p, t, TAG_DATA);
        CheckStatus(mstat);
        board_buffer[t->location[0]][t->location[1]] = TASK_IN_USE;

        MPI_Isend(&send_buffer[send_node].memory, send_buffer_size, MPI_CHAR,
            send_node, intags[send_node], MPI_COMM_WORLD, &send_request[index]);
        MPI_Irecv(&recv_buffer[send_node].memory, recv_buffer_size, MPI_CHAR,
            send_node, intags[send_node], MPI_COMM_WORLD, &recv_request[index]);

        MPI_Wait(&send_request[index], &send_status[index]);
      }
    }
    
    if (index == MPI_UNDEFINED) break;
  }
  
  WriteData(p->board, board_buffer[0]);
  mstat = CheckpointPrepare(m, p, c);
  CheckStatus(mstat);

  mstat = CheckpointProcess(m, p, c);
  CheckStatus(mstat);

/*
  // This part is marked for removal, I am not sure whether it is really needed now, some
  // deep cluster testing is needed here
  cid++;
  CheckpointReset(m, p, c, cid);

  // Receive outstanding data 
  MPI_Waitall(m->mpi_size - 1, send_request, send_status);
  MPI_Waitall(m->mpi_size - 1, recv_request, recv_status);
  
  for (i = 1; i < m->mpi_size; i++) {
    //memcpy(header, &recv_buffer[i].memory, sizeof(int) * (HEADER_SIZE));
    
    c_offset = (i-1)*recv_buffer_size;
 //   memcpy(c->storage->memory + c_offset, &recv_buffer[i].memory, (size_t)recv_buffer_size);

    board_buffer[header[3]][header[4]] = header[2];
  }

  // Process the last checkpoint 
  WriteData(p->board, board_buffer[0]);
  mstat = CheckpointPrepare(m, p, c);
  CheckStatus(mstat);

  mstat = CheckpointProcess(m, p, c);
  CheckStatus(mstat);

*/
  
  /* Terminate all workers */
  for (i = 1; i < m->mpi_size - terminated_nodes; i++) {
    tag = TAG_TERMINATE;
    memcpy(&send_buffer[i].memory, &tag, sizeof(int));

    MPI_Isend(&send_buffer[i].memory, send_buffer_size, MPI_CHAR,
        i, intags[i], MPI_COMM_WORLD, &send_request[i-1]);
    MPI_Irecv(&recv_buffer[i].memory, recv_buffer_size, MPI_CHAR,
        i, intags[i], MPI_COMM_WORLD, &recv_request[i-1]);

  }

  MPI_Waitall(m->mpi_size - 1, send_request, send_status);
  MPI_Waitall(m->mpi_size - 1, recv_request, recv_status);

  /* Finalize */
  CheckpointFinalize(m, p, c);
  TaskFinalize(m, p, t);

  if (intags) free(intags);
  if (send_status) free(send_status);
  if (send_request) free(send_request);
  if (recv_status) free(recv_status);
  if (recv_request) free(recv_request);

  //taddr = send_buffer[1].memory;
  //memcpy(header, &taddr, sizeof(int) * (HEADER_SIZE));
  //printf("header %d %d %d %d\n", header[0], header[1], header[2], header[3]);
  //free(send_buffer[0].memory);
  //free(send_buffer[1].memory);

  if (send_buffer) {
  //  for (i = 0; i < m->mpi_size; i++) {
  //    if (send_buffer[i].memory) Free(&send_buffer[i]);
  //  }
    free(send_buffer);
  }

  if (recv_buffer) {
  //  for (i = 0; i < m->mpi_size; i++) {
  //    if (recv_buffer[i].memory) Free(&recv_buffer[i]);
  //  }
    free(recv_buffer);
  }

  if (board_buffer) {
    FreeIntBuffer(board_buffer);
  }

  return mstat;
}

