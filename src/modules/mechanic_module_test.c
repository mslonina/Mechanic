/**
 * @file
 * The sample Mechanic module, implementation of all API functions
 */
#include "MMechanic2.h"
#include "mechanic_module_test.h"

extern int nbody();

/**
 * Implementation of Init()
 */
int Init(init *i) {
  i->options = 123;
  i->pools = 61;

  return SUCCESS;
}

/**
 * Implementation of Setup()
 */
int Setup(setup *s) {
  s->options[0] = (LRC_configDefaults) {
    .space="test", .name="step", .shortName='\0', .value="0.25", .type=LRC_DOUBLE,
    .description="The time step"};
  s->options[1] = (LRC_configDefaults) {
    .space="test", .name="tend", .shortName='t', .value="1000.0", .type=LRC_DOUBLE,
    .description="The period"};
  s->options[2] = (LRC_configDefaults) {
    .space="test", .name="driver", .shortName='i', .value="1", .type=LRC_INT,
    .description="The driver"};
  s->options[3] = (LRC_configDefaults) {
    .space="pool0", .name="size", .shortName='z', .value="25", .type=LRC_INT,
    .description="The pool size"};
  s->options[4] = (LRC_configDefaults) LRC_OPTIONS_END;

  return SUCCESS;
}

/**
 * Implementation of Storage()
 */
int Storage(pool *p, setup *s) {

  /* Change the pool size (number of tasks) according to the pool id */
/*  if (p->pid == 1) {
    p->board->layout.dim[0] = 13;
    p->board->layout.dim[1] = 13;
  }
  if (p->pid == 2) {
    p->board->layout.dim[0] = 3;
    p->board->layout.dim[1] = 5;
  }
*/
  p->storage[0].layout = (schema) {
    .path = "conditions",
    .rank = 2,
    .dim[0] = 5,
    .dim[1] = 6,
    .use_hdf = 1,
    .storage_type = STORAGE_BASIC,
  };

  p->task->storage[0].layout = (schema) {
    .path = "result",
    .rank = 2,
    .dim[0] = 5,
    .dim[1] = 6,
    .use_hdf = 1,
    .storage_type = STORAGE_LIST,
  };

  return SUCCESS;
}

/**
 * Implementation of PoolPrepare()
 */
int PoolPrepare(pool **all, pool *p, setup *s) {
  int i,j;
  int dims[2];

  dims[0] = p->storage[0].layout.dim[0];
  dims[1] = p->storage[0].layout.dim[1];

  for (j = 0; j < dims[0]; j++) {
    for (i = 0; i < dims[1]; i++) { 
      p->storage[0].data[j][i] = i + j;
    }
  }

  return SUCCESS;
}

/**
 * Implementation of PoolProcess()
 */
int PoolProcess(pool **all, pool *p, setup *s) {
  //int i,j;
  /*for (j = 0; j < p->storage[2].layout.dim[0]; j++) {
    for (i = 0; i < p->storage[2].layout.dim[1]; i++) { 
      printf("%02.1f " , p->storage[2].data[j][i]);
    }
    printf("\n");
  }*/
  printf("pool pid = %d\n", p->pid);

  if (p->pid < 1) return POOL_CREATE_NEW;
  return POOL_FINALIZE;
}

/**
 * Implementation of TaskPrepare()
 */
int TaskPrepare(pool *p, task *t, setup *s) {
  int i,j;
  int dims[6];

  dims[0] = t->storage[0].layout.dim[0];
  dims[1] = t->storage[0].layout.dim[1];

  for (j = 0; j < dims[0]; j++) {
    for (i = 0; i < dims[1]; i++) { 
      t->storage[0].data[j][i] = i + j;
    }
  }

  return SUCCESS;
}

/**
 * Implementation of TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  int i,j,k;

  nbody();

  return SUCCESS;
}

/**
 * Implementation of CheckpointPrepare()
 */
int CheckpointPrepare(pool *p, checkpoint *c, setup *s) {
//  int i = 0;

//  printf("module :: pool %d, cid %d, size = %d\n", p->pid, c->cid, c->size);

//  for (i = 0; i < c->size; i++) {
//    printf("module :: checkpoint %d, data[%d] = %d\n", c->cid, i, c->data[i]);
    //c->data[i] = c->data[i] + 1;
//  }

  return SUCCESS;
}

