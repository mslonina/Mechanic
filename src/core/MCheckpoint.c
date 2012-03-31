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
  int buffer_rank, buffer_dims[MAX_RANK];

  /* Allocate checkpoint pointer */
  c = calloc(sizeof(checkpoint), sizeof(checkpoint));
  if (!c) Error(CORE_ERR_MEM);

  c->cid = cid;
  c->counter = 0;
  c->size = p->checkpoint_size * (m->mpi_size-1);
 
  /* Allocate data buffer */
  buffer_rank = 2;
  
  buffer_dims[0] = c->size;
  buffer_dims[1] = m->mpi_size + 3 + MAX_RANK; // offset: tag, tid, status, location
  for (i = 0; i < m->task_banks; i++) {
    buffer_dims[1] += GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.dim);
  }

  c->data = AllocateBuffer(buffer_rank, buffer_dims);
  if (!c->data) Error(CORE_ERR_MEM);

  return c;
}

/**
 * @function
 * Initialize the checkpoint
 */
int CheckpointInit(module *m, pool *p, checkpoint *c) {
  int mstat = 0;

  return mstat;
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
  int i, j;
  hid_t h5location, group, tasks, datapath;
  hsize_t dims[MAX_RANK], offsets[MAX_RANK];
  char path[LRC_CONFIG_LEN];
  float progress = 0.0;
  task *t;
  int size, position;

  /* Commit data for the task board */
  h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
  sprintf(path, "/Pools/pool-%04d", p->pid);
  group = H5Gopen(h5location, path, H5P_DEFAULT);
  CommitData(group, 1, p->board, STORAGE_BASIC, dims, offsets);

  /* Update pool data */
  CommitData(group, m->pool_banks, p->storage, STORAGE_BASIC, dims, offsets);

  tasks = H5Gopen(group, "Tasks", H5P_DEFAULT);

  t = TaskLoad(m, p, 0);
  position = 5;
  for (j = 0; j < m->task_banks; j++) {
    if (p->task->storage[j].layout.storage_type == STORAGE_PM3D || 
        p->task->storage[j].layout.storage_type == STORAGE_BOARD) {
      
      dims[0] = p->board->layout.dim[0];
      dims[1] = p->board->layout.dim[1];

      for (i = 0; i < c->size; i++) {
        t->status = c->data[i][2];
        t->location[0] = c->data[i][3];
        t->location[1] = c->data[i][4];
        if (t->status != TASK_EMPTY) {

          if (t->storage[j].layout.storage_type == STORAGE_PM3D) {
            offsets[0] = t->location[0] + dims[0]*t->location[1];
            offsets[1] = 0;
          }
          if (t->storage[j].layout.storage_type == STORAGE_BOARD) {
            offsets[0] = t->location[0] * t->storage[j].layout.dim[0];
            offsets[1] = t->location[1] * t->storage[j].layout.dim[1];
          }
          Vec2Array(&c->data[i][position], t->storage[j].data, t->storage[j].layout.rank, t->storage[j].layout.dim);

          CommitData(tasks, 1, &t->storage[j], 
            t->storage[j].layout.storage_type, dims, offsets);
        }
      }
    }
    if (p->task->storage[j].layout.storage_type == STORAGE_BASIC) {
      for (i = 0; i < c->size; i++) {
        t->tid = c->data[i][1];
        t->status = c->data[i][2];
        t->location[0] = c->data[i][3];
        t->location[1] = c->data[i][4];
        if (t->status != TASK_EMPTY) {
          sprintf(path, "task-%04d", t->tid);
          datapath = H5Gopen(tasks, path, H5P_DEFAULT);
          Vec2Array(&c->data[i][position], t->storage[j].data, t->storage[j].layout.rank, t->storage[j].layout.dim);

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

  /* Mark progress */
  for (i = 0; i < c->size; i++) {
    if (c->data[i][2] == TASK_FINISHED) p->progress++;
  }
  progress = 100.0 * p->progress / (float) p->pool_size;
  Message(MESSAGE_CONT,"  %6.2f%% processed\n", progress);

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
    c->data[i][2] = TASK_EMPTY;
  }
}

/**
 * @function
 * Finalize the checkpoint
 */
void CheckpointFinalize(module *m, pool *p, checkpoint *c) {
  int i = 0;
  int buffer_dims[MAX_RANK];
  
  buffer_dims[0] = m->mpi_size - 1;
  buffer_dims[1] = m->mpi_size + 3 + MAX_RANK; // offset: tag, tid, location
  for (i = 0; i < m->task_banks; i++) {
    buffer_dims[1] += GetSize(p->task->storage[i].layout.rank, p->task->storage[i].layout.dim);
  }
  if (c->data) FreeBuffer(c->data, buffer_dims);

  if (c) free(c);
}

