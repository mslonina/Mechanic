/**
 * @file
 * The work loop
 */
#include "M2Wprivate.h"

/**
 * @brief The work loop
 *
 * @param m The module pointer
 *
 * @return 0 on success, error code otherwise
 */
int Work(module *m) {
  int mstat = SUCCESS;
  unsigned int pid = 0;
  pool **p = NULL;
  int pool_create;
  int disable_task_loop = 0;

  double cpu_time;
  clock_t time_in, time_out;
  clock_t taskloop_in, taskloop_out;
  clock_t resetloop_in, resetloop_out;

  hid_t h5location, h5pool, attr_s, attr_d;
  char path[CONFIG_LEN];

  /* M2Prepare the simulation */
  mstat = M2Prepare(m);
  CheckStatus(mstat);

  /**
   * (A) Initialize the pool bank
   */
  p = calloc(m->layer->init->pools, sizeof(pool*));
  if (!p) Error(CORE_ERR_MEM);

  for (pid = 0; pid < m->layer->init->pools; pid++) {
    p[pid] = PoolLoad(m, pid);
  }

  pid = 0; // reset pid
  pool_create = POOL_CREATE_NEW;

  /**
   * (B) The Restart mode
   * We try to recreate the memory layout for each pool and read stored data
   */
  if (m->mode == RESTART_MODE) {
    if (m->node == MASTER) {
      Message(MESSAGE_INFO, "Recreating the pool loop\n");
    }
    mstat = Restart(m, p, &pid);
    CheckStatus(mstat);
    Message(MESSAGE_DEBUG, "Node %d :: PID %d RID %d SID %d SRID %d\n",
        m->node, pid, p[pid]->rid, p[pid]->sid, p[pid]->srid);
  }

  /**
   * (C) The Pool loop
   */
  do {

    // Pool storage
    if (m->mode != RESTART_MODE) Storage(m, p[pid]);

    if (m->node == MASTER) {
      Message(MESSAGE_INFO, "Entering the pool %04d\n", p[pid]->pid);
    }

    time_in = clock();

    do { // The pool reset loop

      resetloop_in = clock();

      if (m->mode != RESTART_MODE) {
        mstat = PoolReset(m, p[pid]);
        CheckStatus(mstat);
      }

      do { // The pool stage loop

        if (m->mode != RESTART_MODE) p[pid]->srid = 0;

        do { // The paol stage reset loop

          mstat = M2NodePrepare(m, p, p[pid]);
          CheckStatus(mstat);

          mstat = PoolPrepare(m, p, p[pid]);
          CheckStatus(mstat);

          mstat = M2LoopPrepare(m, p, p[pid]);
          CheckStatus(mstat);

          // The Task loop
          taskloop_in = clock();

          MReadOption(p[pid], "disable-task-loop", &disable_task_loop);
          if (disable_task_loop == 0) {
            if (m->node == MASTER) {
              mstat = M2Master(m, p[pid]);
              CheckStatus(mstat);
            } else {
              mstat = M2Worker(m, p[pid]);
              CheckStatus(mstat);
            }
          } else {
            if (m->node == MASTER) {
              Message(MESSAGE_INFO, "Task loop has been disabled for the pool %04d\n", p[pid]->pid);
            }
          }

          taskloop_out = clock();
          cpu_time = (double)(taskloop_out - taskloop_in)/CLOCKS_PER_SEC;
          if (m->node == MASTER && m->showtime) Message(MESSAGE_INFO, "Taskloop completed. CPU time: %f\n", cpu_time);

          mstat = M2LoopProcess(m, p, p[pid]);
          CheckStatus(mstat);

          pool_create = PoolProcess(m, p, p[pid]);

          mstat = M2NodeProcess(m, p, p[pid]);
          CheckStatus(mstat);

          p[pid]->srid++;
          if (m->mode == RESTART_MODE) m->mode = NORMAL_MODE;
        } while (pool_create == POOL_STAGE_RESET);

        p[pid]->sid++;
      } while (pool_create == POOL_STAGE);

      resetloop_out = clock();
      cpu_time = (double)(resetloop_out - resetloop_in)/CLOCKS_PER_SEC;
      if (m->node == MASTER && m->showtime) Message(MESSAGE_INFO, "Resetloop %4d completed. CPU time: %f\n", p[pid]->rid, cpu_time);

      p[pid]->rid++;
    } while (pool_create == POOL_RESET);

    time_out = clock();
    cpu_time = (double)(time_out - time_in)/CLOCKS_PER_SEC;

    if (m->node == MASTER) {
      Message(MESSAGE_INFO, "Pool %04d completed. CPU time: %f\n", p[pid]->pid, cpu_time);

      /**
       * Write global pool attributes
       */
      if (m->stats) {
        h5location = H5Fopen(m->filename, H5F_ACC_RDWR, H5P_DEFAULT);

        sprintf(path, POOL_PATH, p[pid]->pid);
        h5pool = H5Gopen2(h5location, path, H5P_DEFAULT);
        H5CheckStatus(h5pool);

        if (H5Aexists(h5pool, "CPU Time [s]") > 0) {
          attr_d = H5Aopen(h5pool, "CPU Time [s]", H5P_DEFAULT);
          H5Awrite(attr_d, H5T_NATIVE_DOUBLE, &cpu_time);
        } else {
          attr_s = H5Screate(H5S_SCALAR);
          attr_d = H5Acreate2(h5pool, "CPU Time [s]", H5T_NATIVE_DOUBLE, attr_s, H5P_DEFAULT, H5P_DEFAULT);
          H5Awrite(attr_d, H5T_NATIVE_DOUBLE, &cpu_time);
          H5Sclose(attr_s);
        }

        H5Aclose(attr_d);
        H5Gclose(h5pool);
        H5Fclose(h5location);
      }
    }

    pid++;

    /* Security check */
    if (pid == m->layer->init->pools) pool_create = POOL_FINALIZE;

    /* Revert to normal mode, after the restarted pool is finished */
  } while (pool_create != POOL_FINALIZE);

  /* M2Process the simulation */
  mstat = M2Process(m, p);
  CheckStatus(mstat);

  /* Finalize the pool bank */
  for (pid = 0; pid < m->layer->init->pools; pid++) {
    PoolFinalize(m, p[pid]);
  }

  free(p);

  return mstat;
}

/**
 * @brief The master node hook
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int M2Master(module *m, pool *p) {
  int mstat = SUCCESS;
  char *err;
  query *q;

  q = (query*) dlsym(m->layer->mode_handler, "Master");
  err = dlerror();
  if (err == NULL) {
    if (q) mstat = q(m, p);
  }

  CheckStatus(mstat);

  return mstat;
}

/**
 * @brief The worker node hook
 *
 * @param m The module pointer
 * @param p The current pool pointer
 *
 * @return 0 on success, error code otherwise
 */
int M2Worker(module *m, pool *p) {
  int mstat = SUCCESS;
  char *err;
  query *q;

  q = (query*) dlsym(m->layer->mode_handler, "Worker");
  err = dlerror();
  if (err == NULL) {
    if (q) mstat = q(m, p);
  }

  CheckStatus(mstat);

  return mstat;
}

