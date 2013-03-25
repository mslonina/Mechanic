/**
 * @file
 * The Mechanic public API
 */
#ifndef MECHANIC_MECHANIC_H
#define MECHANIC_MECHANIC_H

/* Public headers */
#include "M2Cpublic.h"
#include "M2Apublic.h"
#include "M2Epublic.h"
#include "M2Hpublic.h"
#include "M2Spublic.h"
#include "M2Tpublic.h"
#include "M2Ppublic.h"
#include "M2Rpublic.h"
#include "M2Wpublic.h"

int Init(init *i);
int Setup(setup *s);
int Storage(pool *p, void *s);
int PoolPrepare(pool **allpools, pool *current, void *s);
int PoolProcess(pool **allpools, pool *current, void *s);
int TaskBoardMap(pool *p, task *t, void *s);
int BoardPrepare(pool **all, pool *p, task *t, void *s);
int TaskPrepare(pool *p, task *t, void *s);
int TaskProcess(pool *p, task *t, void *s);
int CheckpointPrepare(pool *p, checkpoint *c, void *s);
int Prepare(int mpi_size, int node, char *masterfile, void *s);
int Process(int mpi_size, int node, char *masterfile, pool **all, void *s);
int DatasetPrepare(hid_t h5location, hid_t h5dataset, pool *p, storage *d, void *s);
int DatasetProcess(hid_t h5location, hid_t h5dataset, pool *p, storage *d, void *s);
int NodePrepare(int mpi_size, int node, pool **all, pool *p, void *s);
int NodeProcess(int mpi_size, int node, pool **all, pool *p, void *s);
int LoopPrepare(int mpi_size, int node, pool **all, pool *p, void *s);
int LoopProcess(int mpi_size, int node, pool **all, pool *p, void *s);
int Send(int mpi_size, int node, int dest, int tag, pool *p, void *s);
int Receive(int mpi_size, int node, int sender, int tag, pool *p, void *s, void *buffer);
int Receive(int mpi_size, int node, int sender, int tag, pool *p, void *s, void *buffer);

#endif

