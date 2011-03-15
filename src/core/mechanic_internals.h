/*
 * MECHANIC
 *
 * Copyright (c) 2010-2011, Mariusz Slonina (Nicolaus Copernicus University)
 * All rights reserved.
 *
 * This file is part of MECHANIC code.
 *
 * MECHANIC was created to help solving many numerical problems by providing
 * tools for improving scalability and functionality of the code. MECHANIC was
 * released in belief it will be useful. If you are going to use this code, or
 * its parts, please consider referring to the authors either by the website
 * or the user guide reference.
 *
 * http://git.astri.umk.pl/projects/mechanic
 *
 * Redistribution and use in source and binary forms,
 * with or without modification, are permitted provided
 * that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of the Nicolaus Copernicus University nor the names of
 *   its contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MECHANIC_INTERNALS_H
#define MECHANIC_INTERNALS_H

#if HAVE_CONFIG_H
  #include <config.h>
#endif

#include <inttypes.h> /* for 32 and 64 bits */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/param.h>
#include <signal.h>
#include <fcntl.h>
#include <dirent.h>
#include <math.h>
#include <popt.h>
#include <dlfcn.h>

#include "mpi.h"
#include "hdf5.h"
#include "libreadconfig.h"
#include "libreadconfig_hdf5.h"

#if PLATFORM_DARWIN == 1
  #define WE_ARE_ON_DARWIN 1
  #define LIB_EXT ".dylib"
  #define LIB_ESZ 6
#endif

#if PLATFORM_LINUX == 1
  #define WE_ARE_ON_LINUX 1
  #define LIB_EXT ".so"
  #define LIB_ESZ 3
#endif

#define MECHANIC_NAME PACKAGE_NAME
#define MECHANIC_VERSION PACKAGE_VERSION
#define MECHANIC_AUTHOR PACKAGE_AUTHOR
#define MECHANIC_BUGREPORT PACKAGE_BUGREPORT
#define MECHANIC_URL PACKAGE_URL

#define MECHANIC_CONFIG_FILE_DEFAULT "config"
#define MECHANIC_NAME_DEFAULT "mechanic"
#define MECHANIC_MODULE_DEFAULT "module"
#define MECHANIC_MASTER_PREFIX_DEFAULT "master"
#define MECHANIC_FILE_EXT "h5"
#define MECHANIC_MASTER_FILE_DEFAULT "module-master.h5"
#define MECHANIC_MASTER_SUFFIX_DEFAULT "-master-00.h5"
#define MECHANIC_XRES_DEFAULT "5"
#define MECHANIC_YRES_DEFAULT "5"
#define MECHANIC_METHOD_DEFAULT "0"
#define MECHANIC_CHECKPOINT_DEFAULT "2000"
#define MECHANIC_CHECKPOINTS 3
#define MECHANIC_MODE_DEFAULT "1"
#define MECHANIC_MRL_DEFAULT 4
#define MECHANIC_IRL_DEFAULT 4
#define MECHANIC_MODULE_PREFIX "libmechanic_module_"

#define MECHANIC_FILE 256
#define MECHANIC_FILE_OLD_PREFIX "backup-"
#define MECHANIC_MAXLENGTH MAXPATHLEN

#define MECHANIC_MODULE_SILENT 0
#define MECHANIC_MODULE_WARN 1
#define MECHANIC_MODULE_ERROR 2

#define MECHANIC_MPI_DEST 0
#define MECHANIC_MPI_SOURCE_TAG 0
#define MECHANIC_MPI_DATA_TAG 2
#define MECHANIC_MPI_STANDBY_TAG 49
#define MECHANIC_MPI_RESULT_TAG 59
#define MECHANIC_MPI_TERMINATE_TAG 99

#define MECHANIC_DATASETCONFIG "/config"
#define MECHANIC_DATABOARD "/board"
#define MECHANIC_DATAGROUP "/data"
#define MECHANIC_STATSGROUP "/stats"
#define MECHANIC_DATASETMASTER "master"

#define MECHANIC_MODULE_API 1
#define MECHANIC_MPI_MASTER_NODE 0

#define MECHANIC_TASK_FINISHED 1
#define MECHANIC_TASK_NOTSTARTED 0
#define MECHANIC_TASK_STARTED -1

enum Modes {
  MECHANIC_MODE_MASTERALONE,
  MECHANIC_MODE_FARM,
  MECHANIC_MODE_FARM2,
  MECHANIC_MODE_GRID,
  MECHANIC_MODE_MULTIFARM
#ifdef HAVE_CUDA_H
  MECHANIC_MODE_CUDA,
#endif
} mechanicModes;

/* POPT MODES */
#define MECHANIC_MODE_MASTERALONE_P '0'
#define MECHANIC_MODE_FARM_P '1'
#define MECHANIC_MODE_FARM2_P '2'
#define MECHANIC_MODE_GRID_P '3'
#define MECHANIC_MODE_MULTIFARM_P '4'
#ifdef HAVE_CUDA_H
  #define MECHANIC_MODE_CUDA_P '5'
#endif

/* MODULE ARCHITECTURE FUNCTION HANDLERS */
typedef int (*module_query_void_f) ();
typedef int (*module_query_int_f) ();

/* MECHANIC INTERNALS STRUCT */
typedef struct {
  int mpi_size;
  int node;
  int comm;
  void* module;
  void* handler;
  configData* config;
  moduleInfo* info;
} mechanic_internals;

char* mechanic_module_filename(char* name);
mechanic_internals mechanic_module_open(char* module);
void mechanic_module_close(mechanic_internals module);
mechanic_internals mechanic_internals_init(int node, moduleInfo* m, configData* d);
void mechanic_internals_schema_init(int node, moduleInfo* m, mechanic_internals* internals);
void mechanic_internals_close(mechanic_internals handler);

/* GLOBALS */
char* ConfigFile;
char* datafile;
char* CheckpointFile;
int allopts;
int usage, help, debug, silent;

/* FUNCTION PROTOTYPES */
int map2d(int c, mechanic_internals handler, moduleInfo*, configData* d, int ind[], int** b);

int buildMasterResultsType(int mrl, masterData* md,
    MPI_Datatype* masterResultsType_ptr);

char* mechanic_module_sym_prefix(char* prefix, char* function);
module_query_int_f mechanic_load_sym(mechanic_internals handler, char* function, int type);
module_query_int_f mechanic_sym_lookup(void* modhand, char* md_name, char* function);

int readDefaultConfig(char* inifile, int flag, LRC_configNamespace* head);
int readCheckpointConfig(char* inifile, LRC_configNamespace* head);

int assignConfigValues(configData* d, LRC_configNamespace* head);

int H5writeMaster(hid_t dset, hid_t memspace, hid_t space, moduleInfo *md,
    configData* d, int* coordsarr, MECHANIC_DATATYPE* resultarr);

int H5writeBoard(hid_t dset, hid_t memspace, hid_t space, int* coordsarr);

int H5createMasterDataScheme(hid_t file_id, moduleInfo *md, configData* d);

int manageCheckPoints(configData *d);

int H5readBoard(configData* d, int** board);

int H5writeCheckPoint(moduleInfo *md, configData* d, int check,
    int** coordsarr, MECHANIC_DATATYPE** resultarr);

int atCheckPoint(int check, int** coordsarr, int** board,
    MECHANIC_DATATYPE** resultarr, moduleInfo *md, configData* d);

int mechanic_ups();

void mechanic_welcome();

void clearArray(MECHANIC_DATATYPE*,int);

int mechanic_mode_multifarm(int mpi_size, int node, mechanic_internals handler,
    moduleInfo* md, configData* d);

int mechanic_mode_farm(int mpi_size, int node, mechanic_internals handler,
    moduleInfo* md, configData* d);
int mechanic_mode_masteralone(int mpi_size, int node, mechanic_internals handler,
    moduleInfo* md, configData* d);

#ifdef HAVE_CUDA_H
int mechanic_mode_cuda(int mpi_size, int node, mechanic_internals handler,
    moduleInfo* md, configData* d);
#endif

int mechanic_printConfig(configData* cd, int flag);
int mechanic_copy(char* in, char* out);

int* AllocateIntVec(int x);
double* AllocateDoubleVec(int x);

int** AllocateInt2D(int x, int y);
double** AllocateDouble2D(int x, int y);

int* IntArrayToVec(int** array, int x, int y);
double* DoubleArrayToVec(double** array, int x, int y);

void FreeIntVec(int* vec);
void FreeInt2D(int** pointer, int elems);
void FreeDoubleVec(double* vec);
void FreeDouble2D(double** pointer, int elems);

#define MECHANIC_POPT_AUTOHELP { NULL, '\0', POPT_ARG_INCLUDE_TABLE,\
  mechanic_poptHelpOptions, 0, "Help options:", NULL },

#define MECHANIC_POPT_MODES { NULL, '\0', POPT_ARG_INCLUDE_TABLE,\
  mechanic_poptModes, 0, "Modes:", NULL },

#define MECHANIC_POPT_RESTART { NULL, '\0', POPT_ARG_INCLUDE_TABLE,\
  mechanic_poptRestart, 0, "Restart options:", NULL },

#define MECHANIC_POPT_DEBUG { NULL, '\0', POPT_ARG_INCLUDE_TABLE,\
  mechanic_poptDebug, 0, "Debug/Message interface options:", NULL },

#endif

