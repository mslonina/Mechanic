/**
 * @file
 * The Arnold Web module for Mechanic: The Mechanic part
 *
 * Working code for the Mechanic-0.13 master branch
 */

/**
 * @defgroup module_arnold_web The Arnold Web module
 * @{
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
 * @brief Implements Init()
 */
int Init(init *i) {
  i->options = 24;
  i->banks_per_pool = 2;
  i->banks_per_task = 3;
  i->pools = 25;

  return TASK_SUCCESS;
}

/**
 * @function
 * @brief Implements Setup()
 */
int Setup(setup *s) {

  s->options[0] = (LRC_configDefaults) {
    .space="arnold",
    .name="step",
    .shortName='\0',
    .value="0.25",
    .type=LRC_DOUBLE,
    .description="The time step"
  };
  s->options[1] = (LRC_configDefaults) {
    .space="arnold",
    .name="tend",
    .shortName='\0',
    .value="1000.0",
    .type=LRC_DOUBLE,
    .description="The period"
  };
  s->options[2] = (LRC_configDefaults) {
    .space="arnold",
    .name="xmin",
    .shortName='\0',
    .value="0.8",
    .type=LRC_DOUBLE,
    .description="Minimum x"
  };
  s->options[3] = (LRC_configDefaults) {
    .space="arnold",
    .name="xmax",
    .shortName='\0',
    .value="1.2",
    .type=LRC_DOUBLE,
    .description="Maximum x"
  };
  s->options[4] = (LRC_configDefaults) {
    .space="arnold",
    .name="ymin",
    .shortName='\0',
    .value="0.8",
    .type=LRC_DOUBLE,
    .description="Minimum y"
  };
  s->options[5] = (LRC_configDefaults) {
    .space="arnold",
    .name="ymax",
    .shortName='\0',
    .value="1.2",
    .type=LRC_DOUBLE,
    .description="Maximum y"
  };
  s->options[6] = (LRC_configDefaults) {
    .space="arnold",
    .name="eps",
    .shortName='\0',
    .value="0.0",
    .type=LRC_DOUBLE,
    .description="Minimum perturbation parameter"
  };
  s->options[7] = (LRC_configDefaults) {
    .space="arnold",
    .name="driver",
    .shortName='\0',
    .value="1",
    .type=LRC_INT,
    .description="The driver: 1 - leapfrog, 2 - SABA3"
  };
  s->options[8] = (LRC_configDefaults) {
    .space="arnold",
    .name="eps_interval",
    .value="0.01",
    .type=LRC_DOUBLE,
    .description="The pool perturbation interval"
  };
  s->options[9] = (LRC_configDefaults) {
    .space="arnold",
    .name="epsmax",
    .value="0.04",
    .type=LRC_DOUBLE,
    .description="The maximum perturbation parameter"
  };
  s->options[10] = (LRC_configDefaults) LRC_OPTIONS_END;

  return TASK_SUCCESS;
}

/**
 * @function
 * @brief Implements Storage()
 */
int Storage(pool *p, setup *s) {

  /**
   * Path: /Pools/pool-ID/Tasks/input
   */
  p->task->storage[0].layout = (schema) {
    .path = "input",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 6,
    .use_hdf = 0,
    .storage_type = STORAGE_PM3D,
  };

  /**
   * Path: /Pools/pool-ID/Tasks/result
   */
  p->task->storage[1].layout = (schema) {
    .path = "result",
    .rank = 2,
    .dim[0] = 1,
    .dim[1] = 4,
    .use_hdf = 1,
    .storage_type = STORAGE_PM3D,
  };

  return TASK_SUCCESS;
}

/**
 * @function
 * @brief Implements TaskPrepare()
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
 * @brief Implements TaskProcess()
 */
int TaskProcess(pool *p, task *t, setup *s) {
  double err, xv[6], tend, step, eps, result = 0.0;
  int driver = 0;

  step = LRC_option2double("arnold", "step", s->head);
  step = step*(pow(5,0.5)-1)/2.0;
  tend = LRC_option2double("arnold", "tend", s->head);
  eps = LRC_option2double("arnold", "eps", s->head);

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
 * @function
 * @brief Implements CheckpointPrepare()
 */
int CheckpointPrepare(pool *p, checkpoint *c, setup *s) {

  printf("Pool: %04d, checkpoint %04d processed\n", p->pid, c->cid);

  return TASK_SUCCESS;
}

/** }@ */
