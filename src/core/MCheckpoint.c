/**
 * @file
 * The checkpoint interface 
 */
#include "MCheckpoint.h"

/**
 * @brief Load the checkpoint
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param cid The current checkpoint id
 *
 * @return The checkpoint pointer, NULL otherwise
 */
checkpoint* CheckpointLoad(module *m, pool *p, int cid) {
  int i = 0;
  checkpoint *c = NULL;

  /* Allocate checkpoint pointer */
  c = calloc(1, sizeof(checkpoint));
  if (!c) Error(CORE_ERR_MEM);

  c->storage = calloc(1, sizeof(storage));
  if (!c->storage) Error(CORE_ERR_MEM);

  c->storage->layout = (schema) STORAGE_END;
  c->storage->memory = NULL;

  c->cid = cid;
  c->counter = 0;
  c->size = p->checkpoint_size / (m->mpi_size-1);
  c->size = c->size * (m->mpi_size-1);
  if (c->size == 0) c->size = m->mpi_size-1;

  /* The storage buffer */
  c->storage->layout.rank = 2;
  c->storage->layout.dim[0] = c->size;
  c->storage->layout.dim[1] = HEADER_SIZE; // offset: tag, tid, status, location

  c->storage->layout.size = sizeof(int) * (HEADER_SIZE);
  c->storage->layout.elements = HEADER_SIZE;

  for (i = 0; i < m->task_banks; i++) {
    c->storage->layout.dim[1] +=
      GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.dim);
    c->storage->layout.size +=
      GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.dim) * p->task->storage[i].layout.datatype_size;
    c->storage->layout.elements +=
      GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.dim);
  }

  Message(MESSAGE_DEBUG, "[%s:%d] Checkpoint size %d %d\n", __FILE__, __LINE__,
      c->size, c->size * (int)c->storage->layout.size);

  c->storage->memory = malloc(c->size * c->storage->layout.size);
  if (!c->storage->memory) Error(CORE_ERR_MEM);

  CheckpointReset(m, p, c, 0);

  return c;
}

/**
 * @brief Prepare the checkpoint
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param c The current checkpoint pointer
 *
 * @return 0 on success, error code otherwise
 */
int CheckpointPrepare(module *m, pool *p, checkpoint *c) {
  int mstat = SUCCESS;
  query *q = NULL;
  setup *s = &(m->layer.setup);

  if (m->node == MASTER) {
    q = LoadSym(m, "CheckpointPrepare", LOAD_DEFAULT);
    if (q) mstat = q(p, c, s);
    CheckStatus(mstat);
  }

  return mstat;
}

/**
 * @brief Process the checkpoint
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param c The current checkpoint pointer
 *
 * @return 0 on success, error code otherwise
 */
int CheckpointProcess(module *m, pool *p, checkpoint *c) {
  int mstat = SUCCESS;
  int i = 0, j = 0, k = 0, l = 0, r = 0;
  char path[LRC_CONFIG_LEN];
  int header[HEADER_SIZE];
  int c_offset = 0, d_offset = 0, e_offset = 0, l_offset = 0, k_offset = 0, z_offset = 0;
  int s_offset = 0, r_offset = 0, dim_offset = 0;
  size_t elements;
  task *t;
  hid_t h5location, group, tasks, datapath;
  hsize_t dims[MAX_RANK], offsets[MAX_RANK];

  Backup(m, &m->layer.setup);

  /* Commit data for the task board */
  h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
  H5CheckStatus(h5location);

  sprintf(path, POOL_PATH, p->pid);

  group = H5Gopen(h5location, path, H5P_DEFAULT);
  H5CheckStatus(group);

  mstat = CommitData(group, 1, p->board);
  CheckStatus(mstat);

  /* Update pool data */
  mstat = CommitData(group, m->pool_banks, p->storage);
  CheckStatus(mstat);

  tasks = H5Gopen(group, TASKS_GROUP, H5P_DEFAULT);
  H5CheckStatus(tasks);

  t = TaskLoad(m, p, 0);

  d_offset = 0;

  for (j = 0; j < m->task_banks; j++) {

    if (p->task->storage[j].layout.storage_type == STORAGE_PM3D ||
        p->task->storage[j].layout.storage_type == STORAGE_LIST ||
        p->task->storage[j].layout.storage_type == STORAGE_BOARD) {

      for (l = 0; l < MAX_RANK; l++) {
        dims[l] = p->board->layout.dim[l];
      }

      for (i = 0; i < c->size; i++) {

        /* Get the data header */
        c_offset = i * c->storage->layout.size;

        mstat = CopyData(c->storage->memory + c_offset, header, sizeof(int)*(HEADER_SIZE));
        CheckStatus(mstat);
        
        t->tid = header[1];
        t->status = header[2];
        t->location[0] = header[3];
        t->location[1] = header[4];
        t->location[2] = header[5];

        Message(MESSAGE_DEBUG, "[%s:%d] TASK   %2d %2d %2d location %2d %2d\n", __FILE__, __LINE__,
            header[0], t->tid, t->status, t->location[0], t->location[1]);

        if (t->tid != TASK_EMPTY && t->status != TASK_EMPTY) {

          for (l = 0; l < MAX_RANK; l++) {
            offsets[l] = 0;
          }

          elements = 1;
          for (k = 1; k < t->storage[j].layout.rank; k++) {
            elements *= t->storage[j].layout.dim[k];
          }
          elements *= t->storage[j].layout.datatype_size;

          dim_offset = 1;
          for (k = 2; k < t->storage[j].layout.rank; k++) {
            dim_offset *= t->storage[j].layout.dim[k];
          }

          /* Prepare STORAGE_PM3D */
          if (t->storage[j].layout.storage_type == STORAGE_PM3D) {
            offsets[0] = (t->location[0] + dims[0]*t->location[1]) * t->storage[j].layout.dim[0] 
              + t->location[2]*dims[0]*dims[1]*t->storage[j].layout.dim[0];
            offsets[1] = 0;
            Message(MESSAGE_DEBUG, "[%s:%d] PM3D[%d] task %d %d %d with offsets %d %d %d\n", __FILE__, __LINE__,
                j, t->tid, t->location[0], t->location[1], (int)offsets[0], (int)offsets[1], (int)offsets[2]);
            
            l_offset = elements;
            z_offset = 0;
          }

          /* Prepare STORAGE_LIST */
          if (t->storage[j].layout.storage_type == STORAGE_LIST) {
            offsets[0] = t->tid * t->storage[j].layout.dim[0];
            offsets[1] = 0;
            Message(MESSAGE_DEBUG, "[%s:%d] LIST[%d] task %d %d %d with offsets %d %d %d\n", __FILE__, __LINE__,
                j, t->tid, t->location[0], t->location[1], (int)offsets[0], (int)offsets[1], (int)offsets[2]);
            
            l_offset = elements;
            z_offset = 0;
          }
          
          /* Prepare STORAGE_BOARD */
          if (t->storage[j].layout.storage_type == STORAGE_BOARD) {
            offsets[0] = t->location[0] * t->storage[j].layout.dim[0];
            offsets[1] = t->location[1] * t->storage[j].layout.dim[1];
            offsets[2] = t->location[2] * t->storage[j].layout.dim[2];

            Message(MESSAGE_DEBUG, "[%s:%d] BOARD[%d] task %d %d %d with offsets %d %d %d\n", __FILE__, __LINE__,
                j, t->tid, t->location[0], t->location[1], (int)offsets[0], (int)offsets[1], (int)offsets[2]);
            
          }

          for (l = 0; l < MAX_RANK; l++) {
            t->storage[j].layout.offset[l] = offsets[l];
          }

          c_offset = (int)c->storage->layout.size*i + (int)sizeof(int)*(HEADER_SIZE);
          
          mstat = CopyData(c->storage->memory + c_offset + d_offset, t->storage[j].memory, t->storage[j].layout.size);
          CheckStatus(mstat);

          /* Commit data to the pool */
          if (t->storage[j].layout.storage_type == STORAGE_BOARD) {

            elements = 1;
            for (k = 2; k < t->storage[j].layout.rank; k++) {
              elements *= t->storage[j].layout.dim[k];
            }

            elements *= t->storage[j].layout.datatype_size;
            s_offset = dims[1] * dims[2] * elements * t->storage[j].layout.dim[1];

            for (k = 0; k < t->storage[j].layout.dim[0]; k++) {
              
              k_offset = k * s_offset;
              k_offset += t->location[0] * t->storage[j].layout.dim[0] * s_offset;
              k_offset += t->location[1] * t->storage[j].layout.dim[1] * dims[2] * elements;
              k_offset += t->location[2] * elements;

              for (r = 0; r < t->storage[j].layout.dim[1]; r++) {

                e_offset = r * elements + k * t->storage[j].layout.dim[1] * elements;
                r_offset = r * elements * dims[2] + k_offset;

                mstat = CopyData(t->storage[j].memory + e_offset, p->task->storage[j].memory + r_offset, elements);
                CheckStatus(mstat);
              }
            }
            
          /* For STORAGE_LIST and STORAGE_PM3D it is simpler */
          } else {
            for (k = 0; k < t->storage[j].layout.dim[0]; k++) {
              k_offset = k * l_offset;
              k_offset += t->storage[j].layout.offset[1] * dim_offset * t->storage[j].layout.datatype_size;
              k_offset += t->storage[j].layout.offset[0] * l_offset;
              k_offset += z_offset;

              e_offset = k * elements;
            
              mstat = CopyData(t->storage[j].memory + e_offset, p->task->storage[j].memory + k_offset, elements);
              CheckStatus(mstat);
            }
          }

          /* Commit data to master datafile */
          if (p->task->storage[j].layout.use_hdf) {
            mstat = CommitData(tasks, 1, &t->storage[j]);
            CheckStatus(mstat);
          }
        }
      }
    }

    if (p->task->storage[j].layout.storage_type == STORAGE_GROUP) {
      for (i = 0; i < c->size; i++) {

        /* Get the data header */
        c_offset = (int)c->storage->layout.size * i;
        
        mstat = CopyData(c->storage->memory + c_offset, header, sizeof(int) * (HEADER_SIZE));
        CheckStatus(mstat);

        t->tid = header[1];
        t->status = header[2];
        t->location[0] = header[3];
        t->location[1] = header[4];

        for (l = 0; l < MAX_RANK; l++) {
          t->storage[j].layout.offset[l] = 0;
        }

        if (t->tid != TASK_EMPTY && t->status != TASK_EMPTY) {

          c_offset = (int)c->storage->layout.size*i + (int)sizeof(int)*(HEADER_SIZE);
          mstat = CopyData(c->storage->memory + c_offset + d_offset, t->storage[j].memory, t->storage[j].layout.size);
          CheckStatus(mstat);

          // Commit data to the pool
          p->tasks[t->tid]->tid = t->tid;
          p->tasks[t->tid]->status = t->status;
          p->tasks[t->tid]->location[0] = t->location[0];
          p->tasks[t->tid]->location[1] = t->location[1];

          mstat = CopyData(t->storage[j].memory, p->tasks[t->tid]->storage[j].memory, t->storage[j].layout.size);
          CheckStatus(mstat);

          // Commit data to master datafile
          if (p->task->storage[j].layout.use_hdf) {
            sprintf(path, TASK_PATH, t->tid);
            datapath = H5Gopen(tasks, path, H5P_DEFAULT);
            H5CheckStatus(datapath);
            mstat = CommitData(datapath, 1, &t->storage[j]);
            CheckStatus(mstat);
            H5Gclose(datapath);
          }
        }
      }
    }

    d_offset += (int)p->task->storage[j].layout.size;
  }

  TaskFinalize(m, p, t);

  H5Gclose(tasks);
  H5Gclose(group);
  H5Fclose(h5location);

  return mstat;
}

/**
 * @brief Reset the checkpoint pointer and update the checkpoint id
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param c The current checkpoint pointer
 * @param cid The checkpoint id to use
 */
void CheckpointReset(module *m, pool *p, checkpoint *c, int cid) {
  int header[HEADER_SIZE] = HEADER_INIT;
  int i = 0, c_offset = 0;
  int mstat = SUCCESS;

  c->cid = cid;
  c->counter = 0;

  for (i = 0; i < c->size; i++) {
    c_offset = i * (int)c->storage->layout.size;
    mstat = CopyData(header, c->storage->memory + c_offset, sizeof(int) * (HEADER_SIZE));
    CheckStatus(mstat);
  }
}

/**
 * @brief Finalize the checkpoint
 *
 * @param m The module pointer
 * @param p The current pool pointer
 * @param c The checkpoint pointer
 */
void CheckpointFinalize(module *m, pool *p, checkpoint *c) {
  if (c->storage->memory) free(c->storage->memory);
  if (c) free(c);
}

/**
 * @brief Create incremental backup
 *
 * @param m The module pointer
 * @param s The setup pointer
 *
 * @return 0 on success, error code otherwise
 */
int Backup(module *m, setup *s) {
  int i = 0, b = 0, mstat = SUCCESS;
  char *current_name, *backup_name, iter[4];
  struct stat current;
  struct stat backup;

  b = LRC_option2int("core", "checkpoint-files", s->head);

  for (i = b-2; i >= 0; i--) {
    snprintf(iter, 3, "%02d", i+1);
    backup_name = Name(LRC_getOptionValue("core", "name", s->head), "-master-", iter, ".h5");

    snprintf(iter, 3,"%02d", i);
    current_name = Name(LRC_getOptionValue("core", "name", s->head), "-master-", iter, ".h5");

    if (stat(current_name, &current) == 0) {
      if (stat(backup_name, &backup) < 0) {
        mstat = Copy(current_name, backup_name);
        CheckStatus(mstat);
      } else {
        if (i == 0) {
          mstat = Copy(current_name, backup_name);
          CheckStatus(mstat);
        } else {
          mstat = rename(current_name, backup_name);
          if (mstat < 0) Error(CORE_ERR_CHECKPOINT);
        }
      }
    }
    free(current_name);
    free(backup_name);
  }

  return mstat;
}

