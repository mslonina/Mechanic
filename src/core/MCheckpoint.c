/**
 * @function
 * Checkpoint related stuff
 */

#include "MCheckpoint.h"

/**
 * @function
 */
checkpoint* CheckpointLoad(module *m, pool *p, int cid) {
  checkpoint *c = NULL;
  int i = 0;

  /* Allocate checkpoint pointer */
  c = malloc(sizeof(checkpoint));
  if (!c) Error(CORE_ERR_MEM);

  c->cid = cid;
  c->counter = 0;
  c->size = p->checkpoint_size * m->mpi_size;
 
  /* Allocate tasks bank */
  c->task = malloc((c->size) * sizeof(task*));
  if (!c->task) Error(CORE_ERR_MEM);

  for (i = 0; i < c->size; i++) {
    c->task[i] = TaskLoad(m, p, i);
  } 

  return c;
}

/**
 * @function
 */
int CheckpointInit(module *m, pool *p, checkpoint *c) {
  int mstat = 0;

  return mstat;
}

/**
 * @function
 */
int CheckpointPrepare(module *m, pool *p, checkpoint *c) {
  int mstat = 0;
  //int i = 0;
  query *q;
  setup s = m->layer.setup;
  
  if (m->node == MASTER) {
    //printf("core :: pool %d, cid %d, size = %d\n", p->pid, c->cid, c->size);
    q = LoadSym(m, "CheckpointPrepare", LOAD_DEFAULT);
    if (q) mstat = q(p, c, s);
  }
  
  /*for (i = 0; i < c->size; i++) {
    printf("core :: pool %d, checkpoint %d, data[%d] = %d\n", p->pid, c->cid, i, c->data[i]);
  }*/

  return mstat;
}

/**
 * @function
 */
int CheckpointProcess(module *m, pool *p, checkpoint *c) {
  int mstat = 0;
  query *q;
  setup s = m->layer.setup;
  
  if (m->node == MASTER) {
    q = LoadSym(m, "CheckpointProcess", LOAD_DEFAULT);
    if (q) mstat = q(p, c, s);
  }

  return mstat;
}

/**
 * @function
 */
void CheckpointFinalize(module *m, pool *p, checkpoint *c) {
  int i = 0;
  for (i = 0; i < c->size; i++) {
    TaskFinalize(m, p, c->task[i]);
  }
  if (c->task) free(c->task);
  if (c) free(c);
}
