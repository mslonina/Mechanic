/**
 * C programmers never die. They are just cast into void.
 *
 * Dennis Ritchie (1941-2011)
 */

/**
 * @file
 * The Main function
 */

/**
 * @mainpage
 *
 * @section Requirements Requirements
 *
 * - C compiler (GCC 4.6, Intel 12)
 * - MPI (we favor OpenMPI 1.5)
 * - CMake 2.8
 * - Popt 1.14
 * - LibReadConfig 0.12.4
 *
 * @section gentoo Gentoo Prefix
 *
 *     emerge mechanic
 *
 * @section manual Manual Installation
 *
 *     mkdir build && cd build
 *     CC=mpicc cmake .. -DCMAKE_INSTALL_PREFIX:PATH=/install/prefix/path
 *     make
 *     make install
 *
 * @section features Key features
 *
 * - The Pool-oriented task management, you may define as many task pools as needed, on
 *   the fly (PoolProcess())
 * - Custom setup, both for core and the module, only one config file is used (
 *   Setup()), all setup stored in the master datafile
 * - Custom storage, both for core and the module (Storage())
 * - No MPI and HDF5 knowledge required (i.e. PoolPrepare() does MPI_Broadcast under
 *   the hood)
 * - Non-blocking communication between nodes
 *
 * @section publications Publications
 *
 */
#include "MMain.h"

/**
 * @brief The Main function
 *
 * @param argc The command line argc table
 * @param argv The command line argv table
 *
 * @return EXIT_SUCCESS on success, error code otherwise
 */
int main(int argc, char** argv) {
  int mpi_rank, mpi_size, node;
  module core, module, fallback;

  char *filename, *module_name;
  char *masterfile, *masterfile_backup;

  struct stat file;
  hid_t h5location;

  int mstat = SUCCESS;
  hid_t hstat;

  /**
   * (A) Initialize MPI
   */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  node = mpi_rank;

  /**
   * (B) Initialize HDF
   */
  hstat = H5open();
  H5CheckStatus(hstat);

  if (node == MASTER) Welcome();

  /**
   * (C) Bootstrap core
   */
  core.layer.handler = NULL;
  module.layer.handler = NULL;
  fallback.layer.handler = NULL;

  core = Bootstrap(node, mpi_size, argc, argv, CORE_MODULE, &fallback);
  if (!core.layer.handler) Error(CORE_ERR_CORE); // OOPS! At least Core module should be loaded

  core.mode = NORMAL_MODE;

  /**
   * (D) Handle the restart mode
   */
  if (node == MASTER) {
    if (LRC_option2int("core", "restart-mode", core.layer.setup.head)) {
      core.filename = Name(LRC_getOptionValue("core", "restart-file", core.layer.setup.head), "", "", "");
      Message(MESSAGE_INFO,
          "Switching to restart mode with checkpoint file: '%s'\n", core.filename);
      if (stat(core.filename, &file) < 0) {
        Message(MESSAGE_ERR, "The checkpoint file could not be opened. Aborting\n");
        Abort(CORE_ERR_CHECKPOINT);
      } else {
        if (!H5Fis_hdf5(core.filename)) { // Validate HDF5
          Message(MESSAGE_ERR, "The checkpoint file is not a valid HDF5 file. Aborting\n");
          Abort(CORE_ERR_CHECKPOINT);
        } else { // Validate the Mechanic file
          if (Validate(core.filename) < 0) {
            Message(MESSAGE_ERR, "The checkpoint file is not a valid Mechanic file. Aborting\n");
            Abort(CORE_ERR_CHECKPOINT);
          }
        }
      }
      core.mode = RESTART_MODE;
    }
  }

  MPI_Bcast(&core.mode, 1, MPI_INT, MASTER, MPI_COMM_WORLD);

  /**
   * (E) Core setup
   */
  filename = Name(LRC_getOptionValue("core", "config", core.layer.setup.head), "", "", "");
  mstat = Setup(&core, filename, argc, argv, CORE_SETUP);
  CheckStatus(mstat);
  free(filename);

  if (node == MASTER)
    Message(MESSAGE_INFO, "The core has been bootstrapped and configured\n");

  /**
   * (F) Bootstrap the module
   * Fallback to core if module not specified
   */
  module_name = LRC_getOptionValue("core", "module", core.layer.setup.head);
  module = Bootstrap(node, mpi_size, argc, argv, module_name, &core);
  module.mode = core.mode;
  if (node == MASTER && core.mode == RESTART_MODE) module.filename = Name(core.filename, "", "", "");

  /**
   * (G) Configure the module
   */
  filename = Name(LRC_getOptionValue("core", "config", module.layer.setup.head), "", "", "");

  mstat = Setup(&module, filename, argc, argv, MODULE_SETUP);
  CheckStatus(mstat);
  free(filename);

  if (node == MASTER)
    Message(MESSAGE_INFO, "The '%s' module has been bootstrapped and configured\n", module_name);

  /* Help message */
  if (mstat == CORE_SETUP_HELP) {
    if (node == MASTER) {
      poptPrintHelp(module.layer.setup.popt->poptcontext, stdout, 0);
    }
    goto finalize; // Special help message handling
  }

  if (mstat == CORE_SETUP_USAGE) {
    if (node == MASTER) {
      poptPrintUsage(module.layer.setup.popt->poptcontext, stdout, 0);
    }
    goto finalize; // Special help message handling
  }


  /**
   * (H) Backup the master data file
   */
  if (node == MASTER) {
    if (!LRC_option2int("core", "no-backup", core.layer.setup.head)) {
      masterfile = Name(LRC_getOptionValue("core", "name", core.layer.setup.head),
        "-master-", "00", ".h5");
      masterfile_backup = Name("backup-", masterfile, "", "");

      if (stat(masterfile, &file) == 0) {
        Message(MESSAGE_INFO, "Backup '%s' -> '%s'\n", masterfile, masterfile_backup);
        Copy(masterfile, masterfile_backup);
      }

      free(masterfile);
      free(masterfile_backup);
    }

    /* Copy the restart file to the master file */
    if (module.mode == RESTART_MODE) {
      masterfile = Name(LRC_getOptionValue("core", "name", core.layer.setup.head),
        "-master-", "00", ".h5");
      Copy(module.filename, masterfile);
      free(masterfile);

      /* Our restart file now becomes the master file */
      free(core.filename);
      free(module.filename);
      core.filename = Name(LRC_getOptionValue("core", "name", core.layer.setup.head), "-master-", "00", ".h5");
      module.filename = Name(LRC_getOptionValue("core", "name", core.layer.setup.head), "-master-", "00", ".h5");
    }
  }

  /**
   * (I) Write the configuration to the master file
   */
  if (node == MASTER && module.mode != RESTART_MODE) {
    module.filename = Name(LRC_getOptionValue("core", "name", module.layer.setup.head),
      "-master", "-00", ".h5");

    h5location = H5Fcreate(module.filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    H5CheckStatus(h5location);

    LRC_HDF5Writer(h5location, "/config/core", core.layer.setup.head);
    LRC_HDF5Writer(h5location, "/config/module", module.layer.setup.head);
    H5Fclose(h5location);
  }

  /**
   * (J) Modes
   */
  if (node == MASTER) {
    Message(MESSAGE_INFO, "Entering the pool loop\n");
    Message(MESSAGE_OUTPUT, "\n");
  }

  if (mpi_size > 1) {
    mstat = Taskfarm(&module);
    CheckStatus(mstat);
  } else {
    Message(MESSAGE_WARN, "You must use min. two MPI threads to run Mechanic.\n");
    Message(MESSAGE_WARN, "Try: mpirun -np 2 mechanic2\n");
  }

  /**
   * (K) Finalize
   */

finalize:
  MPI_Barrier(MPI_COMM_WORLD);

  if (module.layer.handler) ModuleFinalize(&module);
  if (core.layer.handler) ModuleFinalize(&core);

  H5close();
  MPI_Finalize();

  if (node == MASTER) {
    Message(MESSAGE_OUTPUT, "\n");
    Message(MESSAGE_INFO, "Mechanic did the job. Have a nice day!\n");
  }

  exit(EXIT_SUCCESS);
}
