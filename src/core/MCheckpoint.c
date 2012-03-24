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

  c->cid = cid;
  c->counter = 0;
  c->size = p->checkpoint_size * (m->mpi_size-1);
 
  /* Allocate tasks bank */
  c->task = calloc((c->size) * sizeof(task*), sizeof(task*));
  if (!c->task) Error(CORE_ERR_MEM);

  for (i = 0; i < c->size; i++) {
    c->task[i] = TaskLoad(m, p, i);
    c->task[i]->status = TASK_EMPTY;
  } 

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
  int i,j,x,y;
  hid_t h5location, group, tasks, datapath;
  hsize_t dims[MAX_RANK], offsets[MAX_RANK];
  char path[LRC_CONFIG_LEN];

  /* Mark board */
  for (i = 0; i < c->size; i++) {
    x = c->task[i]->location[0];
    y = c->task[i]->location[1];
    p->board->data[x][y] = 1.0;
  }

  /* Commit data for the task board */
  h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);
  sprintf(path, "/Pools/pool-%04d", p->pid);
  group = H5Gopen(h5location, path, H5P_DEFAULT);
  CommitData(group, 1, p->board, STORAGE_BASIC, dims, offsets);

  if (!H5Lexists(group, "Tasks", H5P_DEFAULT)) {
    tasks = H5Gcreate(group, "Tasks", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  } else {
    tasks = H5Gopen(group, "Tasks", H5P_DEFAULT);
  }

  /* Commit data for the task */
  for (i = 0; i < c->size; i++) {
    if (c->task[i]->status != TASK_EMPTY) {
      for (j = 0; j < m->task_banks; j++) {
        if (c->task[i]->storage[j].layout.storage_type == STORAGE_BASIC) {
          sprintf(path, "task-%04d", c->task[i]->tid);
          if (!H5Lexists(tasks, path, H5P_DEFAULT)) {
            datapath = H5Gcreate(tasks, path, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
          } else {
            datapath = H5Gopen(tasks, path, H5P_DEFAULT);
          }
        } else {
          datapath = H5Gopen(group, "Tasks", H5P_DEFAULT);

          if (c->task[i]->storage[j].layout.storage_type == STORAGE_PM3D) {
            dims[0] = p->board->layout.dim[0];
            dims[1] = p->board->layout.dim[1];
            offsets[0] = c->task[i]->location[0] + dims[0]*c->task[i]->location[1];
            offsets[1] = 0;
          }
          if (c->task[i]->storage[j].layout.storage_type == STORAGE_BOARD) {
            dims[0] = p->board->layout.dim[0];
            dims[1] = p->board->layout.dim[1];
            offsets[0] = c->task[i]->location[0] * c->task[i]->storage[j].layout.dim[0];
            offsets[1] = c->task[i]->location[1] * c->task[i]->storage[j].layout.dim[1];
          }
          
        }
        CommitData(datapath, 1, &c->task[i]->storage[j], 
            c->task[i]->storage[j].layout.storage_type, dims, offsets);
      }
    }
  }

  H5Gclose(datapath);
  H5Gclose(tasks);
  H5Gclose(group);
  H5Fclose(h5location);

  return mstat;
}

/**
 * @function
 * Finalize the checkpoint
 */
void CheckpointFinalize(module *m, pool *p, checkpoint *c) {
  int i = 0;
  for (i = 0; i < c->size; i++) {
    TaskFinalize(m, p, c->task[i]);
  }
  if (c->task) free(c->task);
  if (c) free(c);
}

