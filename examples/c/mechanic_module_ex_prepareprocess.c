/**
 * Using Prepare/Process hooks
 * ===========================
 *
 * In this example we show how to use Prepare()/Process() hooks. These advanced hooks may
 * be used for additional MPI/HDF5 operations.
 *
 * Compilation
 * -----------
 *
 *    mpicc -std=c99 -fPIC -Dpic -shared -lmechanic -lhdf5 -lhdf5_hl \
 *        mechanic_module_ex_prepareprocess.c -o libmechanic_module_ex_prepareprocess.so
 *
 * Using the module
 * ----------------
 *
 *    mpirun -np 4 mechanic -p ex_prepareprocess -x 10 -y 20
 *
 * Getting the data
 * ----------------
 *
 *    h5dump -d/Pools/pool-0000/Tasks/result mechanic-master-00.h5
 *
 */
#include "mechanic.h"

#define DIM0 4
#define DIM1 4

/**
 * Implements Prepare()
 *
 * This hook is called at the very beginning of the simulation (before the task pool loop
 * begins, but after the master datafile has been created and the simulation setup
 * performed).
 *
 * This hook is called on all nodes
 *
 * Example:
 * We open here the master file, create a simple dataset and write some sample data
 */
int Prepare(int mpi_size, int node, char *masterfile, void *s) {
  hid_t h5location, dataspace, dataset;
  hsize_t dimsf[2];
  double data[DIM0][DIM1];
  int i, j;

  if (node == MASTER) {

    // Buffer initialization
    for (i = 0; i < DIM0; i++) {
      for (j = 0; j < DIM1; j++) {
        data[i][j] = i+j;
      }
    }

    dimsf[0] = DIM0;
    dimsf[1] = DIM1;

    // Open the master datafile
    h5location = H5Fopen(masterfile, H5F_ACC_RDWR, H5P_DEFAULT);
    H5CheckStatus(h5location);

    if (!H5Lexists(h5location, "prepare-dataset", H5P_DEFAULT)) {
      // Create sample dataset and write data into it
      dataspace = H5Screate_simple(2, dimsf, NULL);
      dataset = H5Dcreate2(h5location, "prepare-dataset", H5T_NATIVE_DOUBLE, dataspace, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
      H5Dwrite(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL,H5P_DEFAULT, &data[0][0]);

      // Release the resources
      H5Dclose(dataset);
      H5Sclose(dataspace);
    }
    H5Fclose(h5location);
  }
  return SUCCESS;
}

/**
 * Implements Process()
 *
 * This hook is called at the end of the simulation (after all task pools are processed
 * and stored in the master datafile).
 *
 * This hook is called on all nodes.
 *
 * Example:
 * We open here the master file and read sample data
 */
int Process(int mpi_size, int node, char *masterfile, pool **all, void *s) {
  hid_t h5location, dataset;
  double data[DIM0][DIM1];
  int i, j;

  if (node == MASTER) {
    Message(MESSAGE_COMMENT, "Reading the custom dataset in the Process() hook\n\n");

    // Open the master datafile and the dataset
    h5location = H5Fopen(masterfile, H5F_ACC_RDWR, H5P_DEFAULT);
    dataset = H5Dopen2(h5location, "prepare-dataset", H5P_DEFAULT);

    // Read the whole dataset
    H5Dread(dataset, H5T_NATIVE_DOUBLE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &data[0][0]);

    for (i = 0; i < DIM0; i++) {
      for (j = 0; j < DIM1; j++) {
        Message(MESSAGE_OUTPUT, "%f ", data[i][j]);
      }
      Message(MESSAGE_OUTPUT, "\n");
    }

    // Release the resources
    H5Dclose(dataset);
    H5Fclose(h5location);
  }
  return SUCCESS;
}

