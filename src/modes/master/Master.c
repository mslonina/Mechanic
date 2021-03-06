/**
 * @file
 * The master node (Master mode)
 */
#include "Master.h"

/**
 * Implements Init()
 */
int Init(init *i) {
  i->min_cpu_required = 1;
  return SUCCESS;
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
  int mstat = SUCCESS, ice = 0;
  int k = 0, cid = 0;
  unsigned int c_offset = 0, d_offset = 0, l_size = 0;
  short ****board_buffer = NULL;
  size_t header_size;
  int tag = TAG_TERMINATE;
  int header[HEADER_SIZE] = HEADER_INIT;
  clock_t loop_in, loop_out;
  double cpu_time;

  task *t = NULL;
  checkpoint *c = NULL;

  // Initialize the temporary task board buffer 
  board_buffer = AllocateShort4(p->board);
  ReadData(p->board, &board_buffer[0][0][0][0]);

  if (m->verbose) Message(MESSAGE_INFO, "Completed %04d of %04d tasks\n", p->completed, p->pool_size);

  // Initialize the task and checkpoint 
  t = M2TaskLoad(m, p, 0);
  c = CheckpointLoad(m, p, 0);

  header_size = sizeof(int) * (HEADER_SIZE);

  l_size = header_size;
  for (k = 0; k < p->task_banks; k++) {
    l_size +=
      GetSize(p->task->storage[k].layout.rank, p->task->storage[k].layout.dims) * p->task->storage[k].layout.datatype_size;
  }

  // Specific for the restart mode. The restart file is already full of completed tasks
  if (p->completed == p->pool_size) goto finalize;

  // Start the clock
  loop_in = clock();

  // Do all available tasks on the master node
  while (1) {
    
    mstat = GetNewTask(m, p, t, board_buffer);
    CheckStatus(mstat);
    t->node = MASTER;
    
    if (mstat != NO_MORE_TASKS) {

      do {
        // Check for ICE file
        ice = Ice();
        if (ice == CORE_ICE) {
          Message(MESSAGE_WARN, "The ICE file has been detected. Flushing checkpoints\n");
        }

        // Flush checkpoint buffer and write data, reset counter 
        if ((c->counter > (c->size-1)) || ice == CORE_ICE) {

          WriteData(p->board, &board_buffer[0][0][0][0]);
          mstat = M2CheckpointPrepare(m, p, c);
          CheckStatus(mstat);

          mstat = CheckpointProcess(m, p, c);
          CheckStatus(mstat);

          cid++;

          // Reset the checkpoint
          CheckpointReset(m, p, c, cid);
        }

        // Do simple Abort on ICE
        if (ice == CORE_ICE) Abort(CORE_ICE);

        // Ok, process the task
        board_buffer[t->location[0]][t->location[1]][t->location[2]][0] = TASK_IN_USE;
        if (m->stats) board_buffer[t->location[0]][t->location[1]][t->location[2]][1] = MASTER;
        board_buffer[t->location[0]][t->location[1]][t->location[2]][2] = t->cid;

        mstat = M2TaskPrepare(m, p, t);
        CheckStatus(mstat);

        mstat = M2TaskProcess(m, p, t);
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

        header[0] = tag;
        header[1] = t->tid;
        header[2] = t->status;
        header[3] = t->location[0];
        header[4] = t->location[1];
        header[5] = t->location[2];
        header[6] = t->cid;
      
        c_offset = c->counter * l_size;

        // Write data to the checkpoint buffer
        mstat = CopyData(header, c->storage->memory + c_offset, header_size);
        CheckStatus(mstat);

        d_offset = 0;
        for (k = 0; k < p->task_banks; k++) {
          mstat = CopyData(t->storage[k].memory, c->storage->memory + c_offset + header_size + d_offset, t->storage[k].layout.size);
          CheckStatus(mstat);
          d_offset += t->storage[k].layout.size;
        }
      
        c->counter++;

      } while (t->status != TASK_FINISHED);

      // The task is finished
      board_buffer[header[3]][header[4]][header[5]][0] = t->status;
      if (m->stats) board_buffer[header[3]][header[4]][header[5]][1] = MASTER;
      board_buffer[header[3]][header[4]][header[5]][2] = t->cid;

      if (t->status == TASK_FINISHED) {
        p->completed++;
        
        mstat = M2Receive(MASTER, MASTER, TAG_RESULT, m, p, c->storage->memory);
        CheckStatus(mstat);

        TaskReset(m, p, t, 0);
      }
    } else {
      Message(MESSAGE_DEBUG, "Master: no more tasks after %d of %d completed\n", p->completed, p->pool_size);
    }

    if (p->completed == p->pool_size) break;
  }

  loop_out = clock();
  cpu_time = (double)(loop_out - loop_in)/CLOCKS_PER_SEC;
  if (m->showtime) Message(MESSAGE_INFO, "Computation loop completed. CPU time: %f\n", cpu_time);

  Message(MESSAGE_DEBUG, "Completed %d tasks\n", p->completed);

  WriteData(p->board, &board_buffer[0][0][0][0]);
  mstat = M2CheckpointPrepare(m, p, c);
  CheckStatus(mstat);

  mstat = CheckpointProcess(m, p, c);
  CheckStatus(mstat);

finalize:

  CheckpointFinalize(m, p, c);
  TaskFinalize(m, p, t);

  if (board_buffer) free(board_buffer);

  return mstat;
}
