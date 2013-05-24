/**
 * C programmers never die. They are just cast into void.
 *
 * Dennis Ritchie (1941-2011)
 */

/**
 * @file
 * The Main function
 */
#include "M2Main.h"

/**
 * @brief The Main function
 *
 * @param argc The command line argc table
 * @param argv The command line argv table
 *
 * @return EXIT_SUCCESS on success, error code otherwise
 */
int main(int argc, char **argv) {
  int mpi_rank, mpi_size, node, ice = 0;
  module core, module, fallback;

  char *filename = NULL, *module_name = NULL;
  char *masterfile = NULL, *masterfile_backup = NULL;
  char cwd[MAXPATHLEN+1], hostname[MPI_MAX_PROCESSOR_NAME];
  int hostname_len = MPI_MAX_PROCESSOR_NAME;

  struct stat file;
  hid_t h5location;

  int mstat = SUCCESS;
  hid_t hstat;

  double cpu_time;
  clock_t time_in, time_out;

  hid_t attr_d, attr_s;

  /**
   * (A) Initialize MPI
   */
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
  MPI_Get_processor_name(hostname, &hostname_len);
  
  node = mpi_rank;
  getcwd(cwd,MAXPATHLEN+1);

  /**
   * (B) Initialize HDF
   */
  hstat = H5open();
  H5CheckStatus(hstat);

  if (node == MASTER) Welcome();
  if (node == MASTER) {
    Message(MESSAGE_INFO, "MPI pool size is %d\n", mpi_size);
    Message(MESSAGE_INFO, "The master node is '%s'\n", hostname);
    Message(MESSAGE_INFO, "The current working dir is '%s'\n", cwd);
  }

  /**
   * (C) Bootstrap core
   */
  core.layer.handler = NULL;
  module.layer.handler = NULL;
  fallback.layer.handler = NULL;

  core = Bootstrap(node, mpi_size, argc, argv, CORE_MODULE, &fallback);
  if (!core.layer.handler) {
    Message(MESSAGE_ERR, "Something is screwed! At least Core module should be loaded");
    Error(CORE_ERR_CORE);
  }

  core.mode = NORMAL_MODE;

  /**
   * (D) Handle the restart mode
   */
  if (node == MASTER) {
    if (Option2Int("core", "restart-mode", core.layer.setup.head)) {
      core.filename = Name(Option2String("core", "restart-file", core.layer.setup.head), "", "", "");
      Message(MESSAGE_INFO,
          "Switching to restart mode with checkpoint file: '%s'\n", core.filename);
      if (stat(core.filename, &file) < 0) {
        Message(MESSAGE_ERR, "The checkpoint file could not be opened. Aborting\n");
        Abort(CORE_ERR_RESTART);
      } else {
        if (!H5Fis_hdf5(core.filename)) { // Validate HDF5
          Message(MESSAGE_ERR, "The checkpoint file is not a valid HDF5 file. Aborting\n");
          Abort(CORE_ERR_RESTART);
        } else { // Validate the Mechanic file
          if (Validate(&core, core.filename) < 0) {
            Message(MESSAGE_ERR, "The checkpoint file is not a valid Mechanic file. Aborting\n");
            Abort(CORE_ERR_RESTART);
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
  filename = Name(Option2String("core", "config", core.layer.setup.head), "", "", "");
  mstat = Setup(&core, filename, argc, argv, CORE_SETUP);
  CheckStatus(mstat);
  free(filename);

  if (node == MASTER) {
    Message(MESSAGE_INFO, "The core has been bootstrapped and configured\n");
  }

  /**
   * (F) Bootstrap the module
   * Fallback to core if module not specified
   */
  module_name = Option2String("core", "module", core.layer.setup.head);
  module = Bootstrap(node, mpi_size, argc, argv, module_name, &core);
  module.mode = core.mode;
  
  if (node == MASTER && core.mode == RESTART_MODE) {
    module.filename = Name(core.filename, "", "", "");
  }

  /**
   * (G) Configure the module
   */
  filename = Name(Option2String("core", "config", module.layer.setup.head), "", "", "");

  mstat = Setup(&module, filename, argc, argv, MODULE_SETUP);
  CheckStatus(mstat);
  free(filename);

  if (node == MASTER) {
    Message(MESSAGE_INFO, "The '%s' module has been bootstrapped and configured\n", module_name);
  }

  /* Check for ICE file */
  if (node == MASTER) {
    ice = Ice();
  }

  MPI_Bcast(&ice, 1, MPI_INT, MASTER, MPI_COMM_WORLD);
  
  if (ice == CORE_ICE) {
    if (node == MASTER) {
      Message(MESSAGE_ERR, "The core ICE file '%s' is present. Please remove it to proceed with the simulation\n", ICE_FILENAME);
    }
    goto finalize;
  }

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
    Message(MESSAGE_DEBUG, "(Core)   Name: %s\n", Option2String("core", "name", core.layer.setup.head));
    Message(MESSAGE_DEBUG, "(Module) Name: %s\n", Option2String("core", "name", module.layer.setup.head));
    if (!Option2Int("core", "no-backup", module.layer.setup.head)) {
      masterfile = Name(Option2String("core", "name", module.layer.setup.head), "-master-", "00", ".h5");
      masterfile_backup = Name("backup-", Option2String("core", "name", module.layer.setup.head), "-master-00", ".h5");

      if (stat(masterfile, &file) == 0) {
        Message(MESSAGE_INFO, "Backup '%s' -> '%s'\n\n", masterfile, masterfile_backup);
        Copy(masterfile, masterfile_backup);
      }

      if (masterfile) free(masterfile);
      if (masterfile_backup) free(masterfile_backup);
    }

    /* Copy the restart file to the master file */
    if (module.mode == RESTART_MODE) {
      masterfile = Name(Option2String("core", "name", module.layer.setup.head),
        "-master-", "00", ".h5");
      if (node == MASTER) Message(MESSAGE_DEBUG, "(Restart) Restart file: %s\n", masterfile);
      Copy(module.filename, masterfile);
      free(masterfile);

      // Our restart file now becomes the master file 
      free(core.filename);
      free(module.filename);
      core.filename = Name(Option2String("core", "name", core.layer.setup.head), "-master-", "00", ".h5");
      if (node == MASTER) Message(MESSAGE_DEBUG, "(Restart) Master file: %s\n", module.filename);
      module.filename = Name(Option2String("core", "name", module.layer.setup.head), "-master-", "00", ".h5");
      if (node == MASTER) Message(MESSAGE_DEBUG, "(Restart) Master file: %s\n", module.filename);
    }
  }

  /**
   * (I) Create the master file
   */
  if (node == MASTER && module.mode != RESTART_MODE) {
    core.filename = Name(Option2String("core", "name", module.layer.setup.head),
      "-master", "-00", ".h5");
    module.filename = Name(Option2String("core", "name", module.layer.setup.head),
      "-master", "-00", ".h5");

    h5location = H5Fcreate(module.filename, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
    H5CheckStatus(h5location);

    MechanicHeader(&module, h5location);

    H5Fclose(h5location);
  }

  /**
   * (J) The Work pool
   */

  // Load the runtime mode (no fallback this time)
  module.layer.mode_handler = RuntimeModeLoad(Option2String("core", "mode", module.layer.setup.head));
  if (node == MASTER && module.layer.mode_handler) {
    Message(MESSAGE_INFO, "We are in '%s' mode\n", Option2String("core", "mode", module.layer.setup.head));
  }

  if (mpi_size < module.layer.init.min_cpu_required) {
    Message(MESSAGE_WARN, "You must use min. %d MPI threads to run Mechanic.\n", module.layer.init.min_cpu_required);
    Message(MESSAGE_WARN, "Try: mpirun -np %d mechanic -p %s\n", module.layer.init.min_cpu_required, module_name);
    goto finalize;
  }

  time_in = clock();

  if (node == MASTER) {
    Message(MESSAGE_INFO, "Master file: %s\n", module.filename);
    Message(MESSAGE_INFO, "Entering the pool loop\n");
    Message(MESSAGE_OUTPUT, "\n");
  }

  mstat = Work(&module);
  CheckStatus(mstat);

  time_out = clock();
  cpu_time = (double)(time_out - time_in)/CLOCKS_PER_SEC;

  /**
   * Write global attributes here, such as master cpu_time
   */
  if (node == MASTER && module.mode != RESTART_MODE) {
    if (module.stats) {
      h5location = H5Fopen(module.filename, H5F_ACC_RDWR, H5P_DEFAULT);
    
      attr_s = H5Screate(H5S_SCALAR);
      attr_d = H5Acreate2(h5location, "CPU Time [s]", H5T_NATIVE_DOUBLE, attr_s, H5P_DEFAULT, H5P_DEFAULT);
      H5Awrite(attr_d, H5T_NATIVE_DOUBLE, &cpu_time); 
      H5Aclose(attr_d);
      H5Sclose(attr_s);

      attr_s = H5Screate(H5S_SCALAR);
      attr_d = H5Acreate2(h5location, "MPI size", H5T_NATIVE_INT, attr_s, H5P_DEFAULT, H5P_DEFAULT);
      H5Awrite(attr_d, H5T_NATIVE_INT, &mpi_size); 
      H5Aclose(attr_d);
      H5Sclose(attr_s);

      H5Fclose(h5location);
    }
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
    Message(MESSAGE_INFO, "Master CPU time: %f\n", cpu_time);
  }

  exit(EXIT_SUCCESS);
}

