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
int Storage(pool *p);
int PoolPrepare(pool **allpools, pool *current);
int PoolProcess(pool **allpools, pool *current);
int TaskBoardMap(pool *p, task *t);
int BoardPrepare(pool **all, pool *p, task *t);
int TaskPrepare(pool *p, task *t);
int TaskProcess(pool *p, task *t);
int CheckpointPrepare(pool *p, checkpoint *c);
int Prepare(int mpi_size, int node, char *masterfile);
int Process(int mpi_size, int node, char *masterfile, pool **all);
int DatasetPrepare(hid_t h5location, hid_t h5dataset, pool *p, storage *d);
int DatasetProcess(hid_t h5location, hid_t h5dataset, pool *p, storage *d);
int NodePrepare(int mpi_size, int node, pool **all, pool *p);
int NodeProcess(int mpi_size, int node, pool **all, pool *p);
int LoopPrepare(int mpi_size, int node, pool **all, pool *p);
int LoopProcess(int mpi_size, int node, pool **all, pool *p);
int Send(int mpi_size, int node, int dest, int tag, pool *p);
int Receive(int mpi_size, int node, int sender, int tag, pool *p, void *buffer);
int Receive(int mpi_size, int node, int sender, int tag, pool *p, void *buffer);

#endif

