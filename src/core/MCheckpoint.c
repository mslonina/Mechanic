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
  c->size = p->checkpoint_size * m->mpi_size;
 
  /* Allocate tasks bank */
  c->task = calloc((c->size) * sizeof(task*), sizeof(task*));
  if (!c->task) Error(CORE_ERR_MEM);

  for (i = 0; i < c->size; i++) {
    c->task[i] = TaskLoad(m, p, i);
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
 *
 * @todo
 * - Is this function really needed? Do we have to prepare the checkpoint at the
 *   user-supplied module side?
 */
int CheckpointPrepare(module *m, pool *p, checkpoint *c) {
  int mstat = 0;
  query *q;
  setup s = m->layer.setup;
  
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
  int i,j, x,y;
  query *q;
  setup s = m->layer.setup;
  
  if (m->node == MASTER) {
    q = LoadSym(m, "CheckpointProcess", LOAD_DEFAULT);
    if (q) mstat = q(p, c, s);
  }

  /* Mark board */
  for (i = 0; i < c->size; i++) {
    x = c->task[i]->location[0];
    y = c->task[i]->location[1];
    p->board->data[x][y] = 1.0;
  }

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

