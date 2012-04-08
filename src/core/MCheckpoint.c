/**
 * @function
 * Checkpoint related stuff
 */

#include "MCheckpoint.h"

/**
 * @function
 * Load the checkpoint
 */
checkpoint* CheckpointLoad(module *m, pool *p, int cid) {
  checkpoint *c = NULL;
  int i = 0;

  /* Allocate checkpoint pointer */
  c = calloc(sizeof(checkpoint), sizeof(checkpoint));
  if (!c) Error(CORE_ERR_MEM);

  c->storage = calloc(sizeof(storage), sizeof(storage));
  if (!c->storage) Error(CORE_ERR_MEM);

  c->storage->layout = (schema) STORAGE_END;
  c->storage->data = NULL;

  c->cid = cid;
  c->counter = 0;
  c->size = p->checkpoint_size * (m->mpi_size-1);

  /* The storage buffer */
  c->storage->layout.rank = 2;
  c->storage->layout.dim[0] = c->size;
  c->storage->layout.dim[1] = m->mpi_size + 3 + MAX_RANK; // offset: tag, tid, status, location

  for (i = 0; i < m->task_banks; i++) {
    c->storage->layout.dim[1] += GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.dim);
  }

  c->storage->data = AllocateBuffer(c->storage->layout.rank, c->storage->layout.dim);

  return c;
}

/**
 * @function
 * Prepare the checkpoint
 */
int CheckpointPrepare(module *m, pool *p, checkpoint *c) {
  int mstat = 0;
  query *q;
  setup *s = &(m->layer.setup);

  if (m->node == MASTER) {
    q = LoadSym(m, "CheckpointPrepare", LOAD_DEFAULT);
    if (q) mstat = q(p, c, s);
  }

  return mstat;
}

/**
 * @function
 * Process the checkpoint
 */
int CheckpointProcess(module *m, pool *p, checkpoint *c) {
  int mstat = 0;
  int i, j, k, l;
  hid_t h5location, group, tasks, datapath;
  hsize_t dims[MAX_RANK], offsets[MAX_RANK];
  char path[LRC_CONFIG_LEN];
  task *t;
  int size, position;

  /* Commit data for the task board */
  h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
  sprintf(path, POOL_PATH, p->pid);
  group = H5Gopen(h5location, path, H5P_DEFAULT);
  CommitData(group, 1, p->board, STORAGE_BASIC, dims, offsets);

  /* Update pool data */
  CommitData(group, m->pool_banks, p->storage, STORAGE_BASIC, dims, offsets);

  tasks = H5Gopen(group, TASKS_GROUP, H5P_DEFAULT);

  t = TaskLoad(m, p, 0);
  position = 5;
  for (j = 0; j < m->task_banks; j++) {
    if (p->task->storage[j].layout.storage_type == STORAGE_PM3D ||
        p->task->storage[j].layout.storage_type == STORAGE_LIST ||
        p->task->storage[j].layout.storage_type == STORAGE_BOARD) {

      dims[0] = p->board->layout.dim[0];
      dims[1] = p->board->layout.dim[1];

      for (i = 0; i < c->size; i++) {
        t->tid = c->storage->data[i][1];
        t->status = c->storage->data[i][2];
        t->location[0] = c->storage->data[i][3];
        t->location[1] = c->storage->data[i][4];

        if (t->status != TASK_EMPTY) {

          offsets[0] = 0;
          offsets[1] = 0;

          if (t->storage[j].layout.storage_type == STORAGE_PM3D) {
            offsets[0] = (t->location[0] + dims[0]*t->location[1]) * t->storage[j].layout.dim[0];
            offsets[1] = 0;
          }
          if (t->storage[j].layout.storage_type == STORAGE_LIST) {
            offsets[0] = t->tid * t->storage[j].layout.dim[0];
            offsets[1] = 0;
          }
          if (t->storage[j].layout.storage_type == STORAGE_BOARD) {
            offsets[0] = t->location[0] * t->storage[j].layout.dim[0];
            offsets[1] = t->location[1] * t->storage[j].layout.dim[1];
          }
          Vec2Array(&c->storage->data[i][position], t->storage[j].data, t->storage[j].layout.rank, t->storage[j].layout.dim);

          /* Commit data to the pool */
          for (k = (int)offsets[0]; k < (int)offsets[0] + t->storage[j].layout.dim[0]; k++) {
            for (l = (int)offsets[1]; l < (int)offsets[1] + t->storage[j].layout.dim[1]; l++) {
              p->task->storage[j].data[k][l] = t->storage[j].data[k-(int)offsets[0]][l-(int)offsets[1]];
            }
          }

          /* Commit data to master datafile */
          CommitData(tasks, 1, &t->storage[j],
            t->storage[j].layout.storage_type, dims, offsets);
        }
      }
    }
    if (p->task->storage[j].layout.storage_type == STORAGE_BASIC) {
      for (i = 0; i < c->size; i++) {
        t->tid = c->storage->data[i][1];
        t->status = c->storage->data[i][2];
        t->location[0] = c->storage->data[i][3];
        t->location[1] = c->storage->data[i][4];
        if (t->status != TASK_EMPTY) {
          sprintf(path, TASK_PATH, t->tid);
          datapath = H5Gopen(tasks, path, H5P_DEFAULT);
          Vec2Array(&c->storage->data[i][position], t->storage[j].data, t->storage[j].layout.rank, t->storage[j].layout.dim);

          CommitData(datapath, 1, &t->storage[j],
            t->storage[j].layout.storage_type, dims, offsets);

          H5Gclose(datapath);
        }
      }
    }
    size = GetSize(t->storage[j].layout.rank, t->storage[j].layout.dim);
    position = position + size;
  }
  TaskFinalize(m, p, t);

  H5Gclose(tasks);
  H5Gclose(group);
  H5Fclose(h5location);

  return mstat;
}

/**
 * @function
 * Reset the checkpoint
 */
void CheckpointReset(module *m, pool *p, checkpoint *c, int cid) {

  int i = 0;
  c->cid = cid;
  c->counter = 0;

  for (i = 0; i < c->size; i++) {
    c->storage->data[i][2] = TASK_EMPTY;
  }
}

/**
 * @function
 * Finalize the checkpoint
 */
void CheckpointFinalize(module *m, pool *p, checkpoint *c) {
  if (c->storage->data) FreeBuffer(c->storage->data, c->storage->layout.dim);
  if (c) free(c);
}

