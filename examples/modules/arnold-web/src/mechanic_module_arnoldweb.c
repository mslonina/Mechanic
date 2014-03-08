/**
 * Mechanic module for the Arnold Web
 * ----------------------------------
 *
 * Note: This is a part of the Mechanic package
 *
 * The implementation of the module follows the Arnold Web description from:
 * 
 *  Froeschle C., Guzzo M., Lega E., 
 *  Graphical Evolution of the Arnold Web: From Order to Chaos
 *  Science 289, 2108-2110, 2000
 *
 * Symplectic MEGNO implementation:
 *  
 *  Gozdziewski K., Breiter S., Borczyk W.
 *  The long-term stability of extrasolar system HD37124. 
 *  Numerical study of resonance effects
 *  MNRAS 383, 989-999, 2008
 *
 * SABA integrators family by:
 *
 *  Laskar J., Robutel P.
 *  High order symplectic integrators for perturbed Hamiltonian systems
 *  CMDA 80, 39-62, 2001
 *
 * If you are going to use this code, or its parts, please consider referring to the
 * Authors by the following paper:
 *
 *  Slonina M., Gozdziewski K., Migaszewski C.
 *  "Mechanic: the MPI/HDF code framework for dynamical astronomy"
 *  New Astronomy 2014
 *  http://arxiv.org/abs/1401.6344
 *
 *
 * Module compilation
 * ------------------
 *
 *    mpicc -fPIC -Dpic -shared mechanic_module_arnoldweb.c -o libmechanic_module_arnoldweb.so \
 *      -lmechanic -lhdf5
 *
 * Module options
 * --------------
 *
 *    mpirun -np 4 mechanic -p arnoldweb --help
 *
 * Options:
 * - step - The base time step 
 * - tend - the maximum period
 * - driver - the integrator (1 - Leapfrog, 2 - SABA3, 3 - SABA4)
 * - epsilon_min - the minimum perturbation parameter
 * - epsilon_max - the maximum perturbation parameter
 * - epsilon_interval - the perturbation interval step
 * - configuration-set - the predefined configuration to use:
 *    (-0.5, 1.5, -0.5, 1.5) - The Arnold Web
 *    (0.7, 1.3, 0.7, 1.3)
 *    (0.318, 0.332, 0.096, 0.110)
 *    (0.317, 0.319, 0.096, 0.098)
 *    (0.950, 1.050, 0.950, 1.050)
 *    (0.100, 0.450, 0.100, 0.450)
 *    (0.250, 0.350, 0.250, 0.350)
 *    (0.300, 0.350, 0.150, 0.200)
 *
 * - power-intervals - use power-based time intervals for time-snapshots  
 * - force-step - force the step size (do not use SABA_n initial time-step conversion)
 *
 * Using the module
 * ----------------
 *
 * The module works both in a single dynamical map mode and the pool-based simulation
 * mode. If the epsilon_max is greater than epsilon_min, and the epsilon_interval is
 * specified, the module will run in the pool-based simulation, i.e.
 *
 *    mpirun -np 4 mechanic -p arnoldweb --epsilon-min=0.01 --epsilon-max=0.04 \
 *      --epsilon-interval=0.01
 *
 *
 * Three numerical drivers are provided: the modified leapfrog, SABA3 and SABA4. To use
 * them, use the driver option:
 *
 *    mpirun -np 4 mechanic -p arnoldweb --driver=2 --step-size=0.3
 *
 * By default, the SABA3 and SABA4 integrators convert the initial step size to:
 *
 *    step = step*(pow(5,0.5) - 1.0)/2.0;
 *
 * To force the step-size use force-step option:
 *
 *    mpirun -np 4 mechanic -p arnoldweb --driver=2 --step-size=0.3 --force-step
 *
 * The time-snapshots are computed in the equal time intervals. To switch to power-based
 * time intervals, use power-intervals option:
 *
 *    mpirun -np 4 mechanic -p arnoldweb --power-intervals
 *
 * Maximum intervals is 10. The core task-checkpoints option specifies the numer of
 * checkpoints to store:
 *
 *    mpirun -np 4 mechanic -p arnoldweb --power-intervals --task-checkpoints=7
 *
 * In case of power-based time intervals, the time-snapshots begin with 10^4. The above
 * example will result with time-snapshots up to 10^(4+7)=10^11.
 *
 * The data is stored in the STORAGE_TEXTURE mode and is ready for use with Matplotlib and
 * h5py.
 */
#include "mechanic.h"
#include "mechanic_module_arnoldweb.h"

/* Implements Init() */
int Init(init *i) {
  i->banks_per_pool = 1;
  i->banks_per_task = 2;
  i->attr_per_dataset = 3;
  i->pools = 10;
  i->min_cpu_required = 2;
  return SUCCESS;
}

/* Implements Setup() */
int Setup(setup *s) {
  s->options[0] = (options) {
    .space="aweb", .name="step",
    .shortName='\0', .value="0.25",
    .type=C_DOUBLE,
    .description="The time step"
  };
  s->options[1] = (options) {
    .space="aweb", .name="tend",
    .shortName='\0', .value="1000.0", 
    .type=C_DOUBLE,
    .description="The period"
  };
  s->options[2] = (options) {
    .space="aweb", .name="driver", 
    .shortName='\0', .value="1", 
    .type=C_INT,
    .description="The driver: 1 - Leapfrog, 2 - SABA3, 3 - SABA4"
  };
  s->options[3] = (options) {
    .space="aweb", .name="epsilon_min", 
    .shortName='\0', .value="0.01",
    .type=C_DOUBLE,
    .description="The minimum perturbation parameter"
  };
  s->options[4] = (options) {
    .space="aweb", .name="epsilon_max", 
    .shortName='\0', .value="0.04",
    .type=C_DOUBLE,
    .description="The maximum perturbation parameter"
  };
  s->options[5] = (options) {
    .space="aweb", .name="epsilon_interval", 
    .shortName='\0', .value="0.01", 
    .type=C_DOUBLE,
    .description="The perturbation interval"
  };
  s->options[6] = (options) {
    .space="aweb", .name="configuration-set", 
    .shortName='\0', .value="0", 
    .type=C_INT,
    .description="The configuration set to use"
  };
  s->options[7] = (options) {
    .space="aweb", .name="power-intervals", 
    .shortName='\0', .value="0", 
    .type=C_VAL,
    .description="Use preset intervals"
  };
  s->options[8] = (options) {
    .space="aweb", .name="force-step", 
    .shortName='\0', .value="0", 
    .type=C_VAL,
    .description="Force the step size"
  };

  s->options[9] = (options) OPTIONS_END;

  return SUCCESS;
}

/* Implements Storage() */
int Storage(pool *p) {
  int snapshots;

  MReadOption(p, "task-checkpoints", &snapshots);
  if (snapshots <= 0) snapshots = 1;
  if (snapshots > MAX_SNAPSHOTS) snapshots = MAX_SNAPSHOTS;
  MWriteOption(p, "task-checkpoints", &snapshots);

  // Path: /Pools/pool-ID/sets
  p->storage[0].layout = (schema) {
    .name = "sets",
    .rank = 2,
    .dims[0] = MAX_SETS,
    .dims[1] = 4,
    .use_hdf = HDF_NORMAL_STORAGE,
    .storage_type = STORAGE_GROUP,
    .datatype = H5T_NATIVE_DOUBLE
  };

  // Path: /Pools/pool-ID/Tasks/result
  p->task->storage[0].layout = (schema) {
    .name = "result",
    .rank = TASK_BOARD_RANK + 1,
    .dims[0] = 1,
    .dims[1] = 1,
    .dims[2] = MAX_SNAPSHOTS,
    .dims[3] = 2,
    .use_hdf = HDF_NORMAL_STORAGE, // file storage
    .storage_type = STORAGE_TEXTURE, // texture storage mode
    .datatype = H5T_NATIVE_DOUBLE
  };

  // Time intervals as attributes
  p->task->storage[0].attr[0].layout = (schema) {
    .name = "intervals",
    .rank = 2,
    .dims[0] = 1,
    .dims[1] = MAX_SNAPSHOTS,
    .dataspace = H5S_SIMPLE,
    .datatype = H5T_NATIVE_DOUBLE
  };
  
  // Time interval step
  p->task->storage[0].attr[1].layout = (schema) {
    .name = "interval_step",
    .dataspace = H5S_SCALAR,
    .datatype = H5T_NATIVE_DOUBLE,
  };

  p->task->storage[0].attr[2].layout = (schema) {
    .name = "loop_step",
    .dataspace = H5S_SCALAR,
    .datatype = H5T_NATIVE_ULONG,
  };

  // Temporary storage for the state variables
  p->task->storage[1].layout = (schema) {
    .name = "state",
    .rank = 2,
    .dims[0] = 1,
    .dims[1] = 24,
    .use_hdf = HDF_TEMP_STORAGE,
    .storage_type = STORAGE_LIST,
    .datatype = H5T_NATIVE_DOUBLE
  };

  return SUCCESS;
}

/* Implements PoolPrepare() */
int PoolPrepare(pool **all, pool *p) {
  double eps, epsilon_min, epsilon_interval;
  double xmin, xmax, ymin, ymax;
  int configuration_set;
  double tend, interval_step, intervals[MAX_SNAPSHOTS];
  int i, snapshots, power_intervals;

  // Predefined configurations sets
  double sets[MAX_SETS][4] = {
    {-0.5, 1.5, -0.5, 1.5}, // 0: The Arnold Web
    {0.7, 1.3, 0.7, 1.3}, // 1: A Spider
    {0.318, 0.332, 0.096, 0.110}, // Zoom 2
    {0.317, 0.319, 0.096, 0.098}, // Zoom 3
    {0.950, 1.050, 0.950, 1.050}, // Zoom 4: A Flower
    {0.100, 0.450, 0.100, 0.450}, // 5
    {0.250, 0.350, 0.250, 0.350}, // 6
    {0.300, 0.350, 0.150, 0.200}, // 7
    {0.0, 0.0, 0.0, 0.0},
    {0.0, 0.0, 0.0, 0.0},
  };

  // Write the configuration sets
  MWriteData(p, "sets", &sets[0][0]);

  MReadOption(p, "configuration-set", &configuration_set);
  
  if (configuration_set >= 0) {
    xmin = sets[configuration_set][0];
    xmax = sets[configuration_set][1];
    ymin = sets[configuration_set][2];
    ymax = sets[configuration_set][3];

    MWriteOption(p, "xmin", &xmin);
    MWriteOption(p, "xmax", &xmax);
    MWriteOption(p, "ymin", &ymin);
    MWriteOption(p, "ymax", &ymax);
  } else {
    MReadOption(p, "xmin", &xmin);
    MReadOption(p, "xmax", &xmax);
    MReadOption(p, "ymin", &ymin);
    MReadOption(p, "ymax", &ymax);
  }

  // Prepare time intervals and store them as
  // attributes
  MReadOption(p, "tend", &tend);
  MReadOption(p, "task-checkpoints", &snapshots);
  MReadOption(p, "power-intervals", &power_intervals);

  if (power_intervals == 1) {
    /*
     * 10^4 + 0 = 10^4
     * 10^4 + 1 = 10^5
     * 10^4 + 2 = 10^6
     * 10^4 + 3 = 10^7
     * 10^4 + 4 = 10^8
     * 10^4 + 5 = 10^9
     * 10^4 + 6 = 10^10
     * 10^4 + 7 = 10^11
     * 10^4 + 8 = 10^12
     * 10^4 + 9 = 10^13
     */
    for (i = 0; i < MAX_SNAPSHOTS; i++) {
      intervals[i] = pow(10, i + 4);
    }
    interval_step = 1.0;
  } else { // equal intervals
    interval_step = tend / (double) snapshots;
    for (i = 0; i < snapshots; i++) {
      intervals[i] = (i + 1) * interval_step;
    }
  }

  tend = intervals[snapshots-1];
  MWriteOption(p, "tend", &tend);

  MWriteAttr(p->task, "result", 
      "intervals", &intervals[0]);
  MWriteAttr(p->task, "result", 
      "interval_step", &interval_step);

  MReadOption(p, "epsilon_min", &epsilon_min);
  MReadOption(p, "epsilon_interval", &epsilon_interval);

  if (p->pid > 0) {
    eps = epsilon_min + epsilon_interval * p->pid;
    MWriteOption(p, "epsilon_min", &eps);
  }

  // Print the run summary
  Message(MESSAGE_INFO, "Run summary:\n");
  Message(MESSAGE_OUTPUT, "Tend = %f\n", tend);
  Message(MESSAGE_OUTPUT, "Xrange = (%5f,%5f) Yrange = (%5f,%5f)\n", xmin, xmax, ymin, ymax);
  return SUCCESS;
}

/* Implements PoolProcess() */
int PoolProcess(pool **all, pool *p) {
  double epsilon_min, epsilon_max;

  MReadOption(p, "epsilon_min", &epsilon_min);
  MReadOption(p, "epsilon_max", &epsilon_max);

  if (epsilon_min >= epsilon_max) return POOL_FINALIZE;
  return POOL_CREATE_NEW;
}

/* Implements TaskPrepare() */
int TaskPrepare(pool *p, task *t) {
  double state[24];
  double r[1][1][MAX_SNAPSHOTS][2];
  double xmin, xmax, ymin, ymax;
  double stepx, stepy;
  int i;
  unsigned long int ks;
  
  if (t->cid == 0) {
    // Global map range (equivalent of frequencies space)
    MReadOption(p, "xmin", &xmin);
    MReadOption(p, "xmax", &xmax);
    MReadOption(p, "ymin", &ymin);
    MReadOption(p, "ymax", &ymax);

    stepx = (xmax-xmin)/((double)p->board->layout.dims[1]);
    stepy = (ymax-ymin)/((double)p->board->layout.dims[0]);

    // Initial data
    for (i = 0; i < 24; i++)
      state[i] = 0.0;

    state[3] = xmin + t->location[1]*stepx;
    state[4] = ymin + t->location[0]*stepy;
    state[5] = 1.0;

    // Randomize the tangent vector
    for (i = 6; i < 12; i++)
      state[i] = rand()/(RAND_MAX+1.0);

    MWriteData(t, "state", &state[0]);

    // clear the result buffer
    for (i = 0; i < MAX_SNAPSHOTS; i++) {
      r[0][0][i][0] = 0.0;
      r[0][0][i][1] = 0.0;
    }
    MWriteData(t, "result", &r[0][0][0][0]);

    ks = 0;
    MWriteAttr(t, "result", "loop_step", &ks);
  }

  return SUCCESS;
}

/* Implements TaskProcess() */
int TaskProcess(pool *p, task *t) {
  double state[24];
  double r[1][1][MAX_SNAPSHOTS][2];
  double interval_step;
  double intervals[MAX_SNAPSHOTS];
  double tstart, tend, step, eps;
  double result = 0.0, err = 0.0;
  int driver = 0, task_finalize = 0, i = 0;	
  int snapshots;
  int power_intervals, force_step;
  unsigned long int loop_step;

  MReadOption(p, "step", &step);
  MReadOption(p, "tend", &tend);
  MReadOption(p, "epsilon_min", &eps);
  MReadOption(p, "driver", &driver);
  MReadOption(p, "task-checkpoints", &snapshots);
  MReadOption(p, "power-intervals", &power_intervals);
  MReadOption(p, "force-step", &force_step);

  MReadAttr(t, "result", 
      "intervals", &intervals[0]);
  MReadAttr(t, "result", 
      "interval_step", &interval_step);
  MReadAttr(t, "result",
      "loop_step", &loop_step);

  if (power_intervals == 1) {
    if (t->cid == 0) {
      tstart = 0.0;
    } else {
      tstart = intervals[t->cid-1];
    }
    tend = intervals[t->cid];
  } else { // equal intervals
    tstart = t->cid * interval_step;
    tend = intervals[t->cid];
  }

  if (force_step == 0) {
    step = step*(pow(5,0.5) - 1.0)/2.0;
  }

  MReadData(t, "state", &state[0]);

  if (t->cid > 0) {
    MReadData(t, "result", &r[0][0][0][0]);
  }
 
  // Numerical integration goes here
  if (driver == 1)
    result = smegno2(state, step, &loop_step, tstart, tend, eps, &err);
  
  if (driver == 2)
    result = smegno3(state, step, &loop_step, tstart, tend, eps, &err);
  
  if (driver == 3)
    result = smegno4(state, step, &loop_step, tstart, tend, eps, &err);
 
  // Critical MEGNO check and override
  // Fill up the rest snapshots and finalize the task
  if (result >= CRITICAL_MEGNO) {
    for (i = t->cid; i < snapshots; i++) {
      r[0][0][i][0] = result;
      r[0][0][i][1] = err;
    }

    task_finalize = 1;
  } else {
    // Otherwise, assign the master result
    r[0][0][t->cid][0] = result;
    r[0][0][t->cid][1] = err;
    task_finalize = 0;
  }

  MWriteData(t, "result", &r[0][0][0][0]);
  MWriteAttr(t, "result", "loop_step", &loop_step);

  MWriteData(t, "state", &state[0]);

  if (t->cid + 1 == (unsigned int) snapshots) return TASK_FINALIZE;
  if (task_finalize == 1) return TASK_FINALIZE;
  return TASK_CHECKPOINT;
}

/* Implements Receive() */
int Receive(int mpi_size, int node, int sender, int tag, pool *p, void *buffer) {
  if (node == MASTER) {
    if (tag == TAG_RESULT) {
      if ((p->completed % 1000) == 0) {
        Message(MESSAGE_RESULT, "Completed %6d of %6d tasks\n", p->completed, p->pool_size);
      }
    }
  }
  return SUCCESS;
}

/**
 * The Symplectic MEGNO
 */

/**
 * The right hand sides + variational equations of the Hamiltonian model of the Arnold web, 
 * see Froeschle+ Science 289 (2000)
 */
void vinteraction(double *y,  double *a, 
  double *dy, double *v, double eps) {
  double I1, I2, I3;
  double f1, f2, f3, sf1, sf2;
  double sf3, cf1, cf2, cf3;
  double dif, dif2, dif3, sum;

  f1   = y[0];
  f2   = y[1];
  f3   = y[2];
  I1   = y[3];
  I2   = y[4];
  I3   = y[5];

  sf1  = sin(f1);
  sf2  = sin(f2);
  sf3  = sin(f3);
  cf1  = cos(f1);
  cf2  = cos(f2);
  cf3  = cos(f3);

  dif  = cf1 + cf2 + cf3 + 4.0;
  dif2 = eps/(dif*dif);
  dif3 = dif2/dif;

  // right hand sides
  I3 = 1.0;
  a[0] =  I1;
  a[1] =  I2;
  a[2] =  I3;
  a[3] = -sf1*dif2;
  a[4] = -sf2*dif2;
  a[5] = -sf3*dif2;

  // variational equations
  v[0] =  1.0;
  v[1] =  1.0;
  v[2] =  0.0;

  sum  = 2.0*(sf1*dy[0] + sf2*dy[1] + sf3*dy[2])*dif3;

  v[3] = -cf1*dif2*dy[0] - sum*sf1;
  v[4] = -cf2*dif2*dy[1] - sum*sf2;
  v[5] = -cf3*dif2*dy[2] - sum*sf3;
}

/**
 * The energy integral
 */
double energy(double *y, double eps) {
  double I1, I2, I3, f1, f2, f3;
  double cf1, cf2, cf3, dif, en;

  f1   = y[0];
  f2   = y[1];
  f3   = y[2];
  I1   = y[3];
  I2   = y[4];
  I3   = y[5];

  cf1  = cos(f1);
  cf2  = cos(f2);
  cf3  = cos(f3);

  dif  = eps/(cf1 + cf2 + cf3 + 4.0);
  en   = I1*I1/2.0 + I2*I2/2.0 + I3 + dif;

  return en;
}

/**
 * The variational integral
 */
double variat(double *xv, double *dy, double eps) {
  double acc[6], var[6], vint;

  vinteraction(xv, acc, dy, var, eps);

  vint = -acc[3]*dy[0] -acc[4]*dy[1] -acc[5]*dy[2] +
         +acc[0]*dy[3] +acc[1]*dy[4] +acc[2]*dy[5];

  return vint;
}

/**
 * Normalizes the variational vector (flag = 1)
 */
double norm(int dim, double *a, int flag) {
  double tmp = 0.0;
  int i;

  for (i=0; i<dim; i++) tmp += a[i]*a[i];
  tmp = sqrt(tmp);
  if (flag) for (i=0; i<dim; i++) a[i]= a[i]/tmp;

  return tmp;
}

/**
 * Symplectic MEGNO (Gozdziewski, Breiter & Borczyk, MNRAS, 2008)
 * with the modified Leapfrog integrator SABA2 (Laskar & Robutel, CMDA, 2001)
 */
double smegno2(double *state, double step, unsigned long int *loop_step,
  double tstart, double tend, double eps,
  double *err) {

  double c1, c2, d1;
  double Y0, mY0, Y1, mY1, maxe;
  double acc[6], dy[6], var[6], xv[6];
  double t, en, en0, h, delta, delta0;
  int i, checkout;
  unsigned long int ks;

  c1    = 0.5-sqrt(3.0)/10.0;
  c2    = sqrt(3.0)/3.0;
  d1    = 0.5;

  maxe  = 0.0;
  checkout = 1000;

  // Initialize state vector
  for (i = 0; i < 6; i++) xv[i] = state[i];

  // Set the tangent vector
  for (i = 0; i < 6; i++) dy[i] = state[i+6];

  // State variables
  Y0 = state[12]; mY0 = state[13]; 
  Y1 = state[14]; mY1 = state[15];
  delta0 = state[16]; delta = state[17];
  en0 = state[18]; maxe = state[19];
  tstart = state[20]; 

  if (tstart == 0.0) {
    // Normalize the tangent vector
    delta0= norm(6, dy, 1);
    en0   = energy(xv, eps);
  }

  ks = *loop_step;
  t = tstart;
  while (t <= tend) {

    // 1st drift
    h     = c1*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 1st kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d1*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // 2nd drift
    h     = c2*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    ks++;
    t = ks * step;

    // MEGNO
    delta   = norm(6, dy, 0);
    Y1      =  Y0*((double)ks-1.0)/((double)ks)
                + 2.0*log(delta/delta0);
    mY1     = mY0*((double)ks-1.0)/((double)ks)
                + Y1/((double)ks);

    Y0      = Y1;
    mY0     = mY1;
    delta0  = delta;

    // Relative error of the energy 
    if (ks%checkout == 0) {
      en = fabs((energy(xv,eps)-en0)/en0);
      if (en>maxe) maxe = en;
    }

    if (isnan(mY1)) mY1 = CRITICAL_MEGNO;
    if (mY1 >= CRITICAL_MEGNO) break;
  }

  // Assign the state vector back
  for (i = 0; i < 6; i++) state[i] = xv[i];
  for (i = 0; i < 6; i++) state[i+6] = dy[i];
  state[12] = Y0; state[13] = mY0; 
  state[14] = Y1; state[15] = mY1;
  state[16] = delta0; state[17] = delta;
  state[18] = en0; state[19] = maxe;
  state[20] = t; 

  //printf("ts = %f\n", ts);

  *loop_step = ks;
  *err = maxe;
  return mY1;
}

/**
 * Symplectic MEGNO (Gozdziewski, Breiter & Borczyk, MNRAS, 2008)
 * with the SABA3 integrator (Laskar & Robutel, CMDA, 2001)
 */
double smegno3(double *state, double step, unsigned long int *loop_step,
  double tstart, double tend, double eps, 
  double *err) {

  double c1, c2, d1, d2;
  double Y0, mY0, Y1, mY1, maxe;
  double acc[6], dy[6], var[6], xv[6];
  double t, en, en0, h, delta, delta0;
  unsigned long int ks;
  int i, checkout;

  // SABA3 coefficients
  c1    = 0.5 - sqrt(15.0)/10.0;
  c2    = sqrt(15.0)/10.0;
  d1    = 5.0/18.0;
  d2    = 4.0/9.0;

  maxe  = 0.0;
  checkout = 1000;

  // Initialize state vector
  for (i = 0; i < 6; i++) xv[i] = state[i];

  // Set the tangent vector
  for (i = 0; i < 6; i++) dy[i] = state[i+6];

  // State variables
  Y0 = state[12]; mY0 = state[13]; 
  Y1 = state[14]; mY1 = state[15];
  delta0 = state[16]; delta = state[17];
  en0 = state[18]; maxe = state[19];
  tstart = state[20];

  if (tstart == 0.0) {
    // Normalize the tangent vector
    delta0= norm(6, dy, 1);
    en0   = energy(xv, eps);
  }

  ks = *loop_step;
  t = tstart;
  while (t <= tend) {

    // 1st drift
    h     = c1*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 1st kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d1*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // 2nd drift
    h     = c2*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 2nd kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d2*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // 3rd drift
    h     = c2*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 3rd kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d1*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // final drift
    h     = c1*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    ks++;
    t = ks*step;

    // MEGNO
    delta   = norm(6, dy, 0);
    Y1      =  Y0*((double)ks-1.0)/((double)ks) 
      + 2.0*log(delta/delta0);
    mY1     = mY0*((double)ks-1.0)/((double)ks) 
      + Y1/((double)ks);
    Y0      = Y1;
    mY0     = mY1;
    delta0  = delta;

    // relative error of the energy
    if (ks%checkout == 0) {
      en = fabs((energy(xv,eps)-en0)/en0);
      if (en>maxe) maxe = en;
    }
    
    if (isnan(mY1)) mY1 = CRITICAL_MEGNO;
    if (mY1 >= CRITICAL_MEGNO) break;
  }

  // Assign the state vector back
  for (i = 0; i < 6; i++) state[i] = xv[i];
  for (i = 0; i < 6; i++) state[i+6] = dy[i];
  state[12] = Y0; state[13] = mY0; 
  state[14] = Y1; state[15] = mY1;
  state[16] = delta0; state[17] = delta;
  state[18] = en0; state[19] = maxe;
  state[20] = t; 

  *loop_step = ks;
  *err = maxe;
  return mY1;
}

/**
 * Symplectic MEGNO (Gozdziewski, Breiter & Borczyk, MNRAS, 2008)
 * with the SABA4 integrator (Laskar & Robutel, CMDA, 2001)
 */
double smegno4(double *state, double step, unsigned long int *loop_step,
  double tstart, double tend, double eps, 
  double *err) {

  double c1, c2, c3, d1, d2, lcp, lcm;
  double Y0, mY0, Y1, mY1, maxe;
  double acc[6], dy[6], var[6], xv[6];
  double t, en, en0, h, delta, delta0;
  unsigned long int ks;
  int i, checkout;

  // SABA4 coefficients
  lcp = sqrt(525.0+70.0*sqrt(30.0));
  lcm = sqrt(525.0-70.0*sqrt(30.0));

  c1    = 0.5 - lcp/70.0;
  c2    = (lcp-lcm)/70.0;
  c3    = lcm/35.0;
  d1    = 1.0/4.0 - sqrt(30.0)/72.0;
  d2    = 1.0/4.0 + sqrt(30.0)/72.0;

  maxe  = 0.0;
  checkout = 1000;

  // Initialize state vector
  for (i = 0; i < 6; i++) xv[i] = state[i];

  // Set the tangent vector
  for (i = 0; i < 6; i++) dy[i] = state[i+6];

  // State variables
  Y0 = state[12]; mY0 = state[13]; 
  Y1 = state[14]; mY1 = state[15];
  delta0 = state[16]; delta = state[17];
  en0 = state[18]; maxe = state[19];
  tstart = state[20];

  if (tstart == 0.0) {
    // Normalize the tangent vector
    delta0= norm(6, dy, 1);
    en0   = energy(xv, eps);
  }

  ks = *loop_step;
  t = tstart;
  while (t <= tend) {

    // 1st drift
    h     = c1*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 1st kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d1*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // 2nd drift
    h     = c2*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 2nd kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d2*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // 3rd drift
    h     = c3*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 3rd kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d2*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // 3rd drift
    h     = c2*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    // 3rd kick
    vinteraction(xv, acc, dy, var, eps);
    h     = d1*step;
    xv[3] = xv[3] + acc[3]*h;
    xv[4] = xv[4] + acc[4]*h;
    xv[5] = xv[5] + acc[5]*h;

    dy[3] = dy[3] + var[3]*h;
    dy[4] = dy[4] + var[4]*h;
    dy[5] = dy[5] + var[5]*h;

    // final drift
    h     = c1*step;
    xv[0] = xv[0] + xv[3]*h;
    xv[1] = xv[1] + xv[4]*h;
    xv[2] = xv[2] + 1.0*h;

    dy[0] = dy[0] + dy[3]*h;
    dy[1] = dy[1] + dy[4]*h;
    dy[2] = dy[2];

    ks++;
    t = ks*step;

    // MEGNO
    delta   = norm(6, dy, 0);
    Y1      =  Y0*((double)ks-1.0)/((double)ks) 
      + 2.0*log(delta/delta0);
    mY1     = mY0*((double)ks-1.0)/((double)ks) 
      + Y1/((double)ks);
    Y0      = Y1;
    mY0     = mY1;
    delta0  = delta;

    // relative error of the energy
    if (ks%checkout == 0) {
      en = fabs((energy(xv,eps)-en0)/en0);
      if (en>maxe) maxe = en;
    }
    
    if (isnan(mY1)) mY1 = CRITICAL_MEGNO;
    if (mY1 >= CRITICAL_MEGNO) break;
  }

  // Assign the state vector back
  for (i = 0; i < 6; i++) state[i] = xv[i];
  for (i = 0; i < 6; i++) state[i+6] = dy[i];
  state[12] = Y0; state[13] = mY0; 
  state[14] = Y1; state[15] = mY1;
  state[16] = delta0; state[17] = delta;
  state[18] = en0; state[19] = maxe;
  state[20] = t; 

  *loop_step = ks;
  *err = maxe;
  return mY1;
}

