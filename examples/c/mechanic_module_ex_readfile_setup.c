/**
 * Reading input file in PoolPrepare() with Setup()
 * ================================================
 *
 * In this example we read sample text file during PoolPrepare(). The data is stored in
 * the pool storage bank and synced to all worker nodes (`sync = 1`), right after the
 * PoolPrepare() is finished. The default file "readfile.txt" may be changed through the
 * defined option "inputfile":
 *
 *    mpirun -np 2 mechanic -p ex_readfile_setup --inputfile=input.dat
 *
 *    or
 *    
 *    mpirun -np 2 mechanic -p ex_readfile_setup -f input.dat
 *
 * The data can be accessed in `/Pools/pool-0000/input` dataset.
 *
 * The module returns `MODULE_SETUP_ERR` in case the datafile could not be read.
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_readfile_setup.c -o libmechanic_module_ex_readfile_setup.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_readfile_setup -x 10 -y 20
 *
 */
#include "mechanic.h"

#define MAX_ENTRIES 4
#define ENTRY_LENGTH 7
#define DATAFILE "readfile.txt"

int ReadFile(char *filename, double **buffer);

/**
 * Very simple function to read a file
 */
int ReadFile(char *filename, double **buffer) {
  FILE *fp;
  char line[1024];
  int i;

  fp = fopen(filename, "r");
  if (fp) {
    Message(MESSAGE_INFO, "Reading input file '%s'\n", filename);
    i = 0;
    while (fgets(line, 1024, fp)) {
      if (line[0] == '#') continue; // very simple comment handling
      if (i > MAX_ENTRIES) break; // very simple max entries handling

      sscanf(line, "%lf %lf %lf %lf %lf %lf %lf",
          &buffer[i][0], &buffer[i][1], &buffer[i][2], &buffer[i][3],
          &buffer[i][4], &buffer[i][5], &buffer[i][6]);
      i++;
    }
    fclose(fp);
  } else {
    Message(MESSAGE_ERR, "The input file '%s' could not be loaded\n", filename);
    return MODULE_ERR_SETUP;
  }
  return SUCCESS;
}

/**
 * Implements Setup()
 */
int Setup(setup *s) {
  s->options[0] = (options) {
    .space = "ex_readfile",
    .name = "inputfile",
    .shortName = 'f',
    .value = DATAFILE,
    .type = C_STRING,
    .description = "The input file to read"
  };
  s->options[1] = (options) OPTIONS_END;
  return SUCCESS;
}

/**
 * Implements Storage()
 */
int Storage(pool *p, void *s) {
  p->storage[0].layout = (schema) {
    .name = "input",
    .rank = 2,
    .dims[0] = MAX_ENTRIES,
    .dims[1] = ENTRY_LENGTH,
    .datatype = H5T_NATIVE_DOUBLE,
    .storage_type = STORAGE_GROUP,
    .use_hdf = 1,
    .sync = 1
  };
  return SUCCESS;
}

/**
 * Implements PoolPrepare()
 */
int PoolPrepare(pool **all, pool *p, void *s) {
  double **buffer;
  int mstat = SUCCESS;
  char filename[CONFIG_LEN];

  MAllocate2(p, "input", buffer, double);

  MReadOption(p, "inputfile", &filename);
  mstat = ReadFile(filename, buffer);

  MWriteData(p, "input", &buffer[0][0]);

  free(buffer);
  return mstat;
}

