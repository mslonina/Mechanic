set (ENV{LD_LIBRARY_PATH} ".")
set (ENV{DYLD_LIBRARY_PATH} ".")

execute_process(COMMAND mpirun -np 4 mechanic -p ${MODULE} -n ${MODULE} OUTPUT_VARIABLE
  TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (EOUT) 
  message(STATUS ${TOUT})
  message(STATUS ${ROUT})
  message(FATAL_ERROR ${EOUT})
endif (EOUT)

