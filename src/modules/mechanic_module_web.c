/**
 * @file
 * The Arnold Web module for Mechanic: The Mechanic part
 * 
 * Working code for the Mechanic-0.13 master branch
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/**
 * Include Mechanic datatypes and function prototypes
 */
#include "MMechanic2.h"

/**
 * Include module-related stuff
 */
#include "mechanic_module_web.h"

/**
 * @function
 * Implemens Setup()
 */
int Setup(setup *s) {

  s->options[0] = (LRC_configDefaults) {"arnold", "step", "0.25", LRC_DOUBLE};
  s->options[1] = (LRC_configDefaults) {"arnold", "tend", "1000.0", LRC_DOUBLE};
  s->options[2] = (LRC_configDefaults) {"arnold", "xmin", "0.8", LRC_DOUBLE};
  s->options[3] = (LRC_configDefaults) {"arnold", "xmax", "1.2", LRC_DOUBLE};
  s->options[4] = (LRC_configDefaults) {"arnold", "ymin", "0.8", LRC_DOUBLE};
  s->options[5] = (LRC_configDefaults) {"arnold", "ymax", "1.2", LRC_DOUBLE};
  s->options[6] = (LRC_configDefaults) {"arnold", "eps", "0.01", LRC_DOUBLE};
  s->options[7] = (LRC_configDefaults) {"arnold", "driver", "1", LRC_INT};
  s->options[8] = (LRC_configDefaults) {LRC_OPTIONS_END};

  return TASK_SUCCESS;
}

/**
 * Implements Storage()
 */
int Storage(pool *p, setup *s) {
  
  /* Pools/pool-D/Tasks/input */
  p->task->storage[0].layout.path = "input";
  p->task->storage[0].layout.rank = 2;
  p->task->storage[0].layout.dim[0] = 1;
  p->task->storage[0].layout.dim[1] = 6;
  p->task->storage[0].layout.use_hdf = 1;
  p->task->storage[0].layout.dataspace_type = H5S_SIMPLE;
  p->task->storage[0].layout.datatype = H5T_NATIVE_DOUBLE;
  p->task->storage[0].layout.storage_type = STORAGE_PM3D;

  /* Pools/pool-D/Tasks/result */
  p->task->storage[1].layout.path = "result";
  p->task->storage[1].layout.rank = 2;
  p->task->storage[1].layout.dim[0] = 1;
  p->task->storage[1].layout.dim[1] = 4;
  p->task->storage[1].layout.use_hdf = 1;
  p->task->storage[1].layout.dataspace_type = H5S_SIMPLE;
  p->task->storage[1].layout.datatype = H5T_NATIVE_DOUBLE;
  p->task->storage[1].layout.storage_type = STORAGE_PM3D;

  return TASK_SUCCESS;
}

/**
 * @function
 * Implements TaskPrepare()
 */
int TaskPrepare(pool *p, task *t, setup *s) {
  double xmin, xmax, ymin, ymax;

  /* Global map range (equivalent of frequencies space) */
  xmin = LRC_option2double("arnold", "xmin", s->head);
  xmax = LRC_option2double("arnold", "xmax", s->head);
  ymin = LRC_option2double("arnold", "ymin", s->head);
  ymax = LRC_option2double("arnold", "ymax", s->head);

  /* Initial condition - angles */
  t->storage[0].data[0][0] = 0.131;
  t->storage[0].data[0][1] = 0.132;
  t->storage[0].data[0][2] = 0.212;

  /* Map coordinates */  
  t->storage[0].data[0][3] = xmin + t->location[1]*(xmax-xmin)/(1.0*p->board->layout.dim[1]);
  t->storage[0].data[0][4] = ymin + t->location[0]*(ymax-ymin)/(1.0*p->board->layout.dim[0]);
  t->storage[0].data[0][5] = 0.01;  

  return TASK_SUCCESS;
}

/**
 * @function
 * Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double err, xv[6], tend, step, eps, result;
  int driver;

  step  = 0.25*(pow(5,0.5)-1)/2.0;
  tend  = 20000.0;
  eps   = 0.01;

  step = LRC_option2double("arnold", "step", s->head);
  step  = step*(pow(5,0.5)-1)/2.0;
  tend = LRC_option2double("arnold", "tend", s->head);
  eps  = LRC_option2double("arnold", "eps", s->head);
  driver = LRC_option2int("arnold", "driver", s->head);

  /* Initial data */  
  xv[0] = t->storage[0].data[0][0];
  xv[1] = t->storage[0].data[0][1];
  xv[2] = t->storage[0].data[0][2];
  xv[3] = t->storage[0].data[0][3];
  xv[4] = t->storage[0].data[0][4];
  xv[5] = t->storage[0].data[0][5];  

  /* Numerical integration goes here */
  if (driver == 1) result = smegno2(xv, step, tend, eps, &err);
  if (driver == 2) result = smegno3(xv, step, tend, eps, &err);

  /* Assign the master result */
  t->storage[1].data[0][0] = xv[3];
  t->storage[1].data[0][1] = xv[4];
  t->storage[1].data[0][2] = result;
  t->storage[1].data[0][3] = err;

  return TASK_SUCCESS;
}
 
/**
 * Implements PoolProcess()
 */
int PoolProcess(pool *p, setup *s) {
  return POOL_FINALIZE;
}
