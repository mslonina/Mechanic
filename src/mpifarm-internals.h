#ifndef MPIFARM_INTERNALS_H
#define MPIFARM_INTERNALS_H

/* GLOBALS */
char* inifile;
char* datafile;
int allopts, mpi_rank, mpi_size;
int usage, help;

/* FUNCTION PROTOTYPES */
int* map2d(int, void* handler, moduleInfo*, configData* d);
void master(void* handler, moduleInfo*, configData* d, int restartmode);
void slave(void* handler, moduleInfo*, configData* d);
void clearArray(MECHANIC_DATATYPE*,int);
void buildMasterResultsType(int mrl, masterData* md, MPI_Datatype* masterResultsType_ptr);
void buildDefaultConfigType(configData* d, MPI_Datatype* defaultConfigType_ptr);
void* load_sym(void* handler, moduleInfo*, char* function, int type);
int readDefaultConfig(char* inifile, LRC_configNamespace* cs, LRC_configTypes* ct, int numCT, int flag);
void assignConfigValues(int opts, configData* d, LRC_configNamespace* cs, int flag, int popt);
void H5writeMaster(hid_t dset, hid_t memspace, hid_t space, configData* d, int* coordsarr, MECHANIC_DATATYPE* resultarr);
void H5writeBoard(hid_t dset, hid_t memspace, hid_t space, int* coordsarr);
void mpi_displayArgs(poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data);
void mpi_displayUsage(poptContext con, enum poptCallbackReason reason, const struct poptOption* key, 
    char* arg, void* data);
void poptTestC(char* i, char* j);
void poptTestI(char* i, int j);
void welcome();
void H5createMasterDataScheme(hid_t file_id, configData* d);
void H5writeCheckPoint(configData* d, int check, int** coordsarr, MECHANIC_DATATYPE** resultarr);
void H5readBoard(configData* d, int** board);

#define MPI_POPT_AUTOHELP { NULL, '\0', POPT_ARG_INCLUDE_TABLE, mpi_poptHelpOptions, \
			0, "Help options:", NULL },


#endif
