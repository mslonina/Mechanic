set (ENV{LD_LIBRARY_PATH} $ENV{LD_LIBRARY_PATH}:.)
set (ENV{DYLD_LIBRARY_PATH} $ENV{DYLD_LIBRARY_PATH}:.)

message(STATUS "Mechanic path is: ${MECHANIC}")

#
# Task farm: The normal mode
#
message(STATUS "Testing normal mode (task farm)")
execute_process(COMMAND mpirun -np 4 ${MECHANIC} -p ${MODULE} -n ${MODULE} -x 10 -y 10 -b 3 -d 13 --test
  --restart-file=${MODULE}-master-02.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (EOUT) 
  message(STATUS ${TOUT})
  message(STATUS ${ROUT})
  message(FATAL_ERROR ${EOUT})
endif (EOUT)

execute_process(COMMAND h5diff ${MODULE}-master-00.h5 references/${MODULE}-master-00.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (TOUT)
  message(FATAL_ERROR ${TOUT})
endif (TOUT)

#
# Task farm: The restart mode
#
message(STATUS "Testing restart mode (task farm)")
execute_process(COMMAND 
  mpirun -np 4 ${MECHANIC} -p ${MODULE} -n ${MODULE} -x 10 -y 10 -b 3 -d 13 --test
  --restart-mode --restart-file=${MODULE}-master-02.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (EOUT) 
  message(STATUS ${TOUT})
  message(STATUS ${ROUT})
  message(FATAL_ERROR ${EOUT})
endif (EOUT)

execute_process(COMMAND h5diff ${MODULE}-master-00.h5 references/${MODULE}-master-00.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (TOUT)
  message(FATAL_ERROR ${TOUT})
endif (TOUT)

#
# Master: The normal mode
#
message(STATUS "Testing normal mode (master)")
execute_process(COMMAND 
  mpirun -np 4 ${MECHANIC} -m master -p ${MODULE} -n ${MODULE}-master-mode -x 10 -y 10 -b 3 -d 13 --test
  --restart-file=${MODULE}-master-mode-master-02.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (EOUT) 
  message(STATUS ${TOUT})
  message(STATUS ${ROUT})
  message(FATAL_ERROR ${EOUT})
endif (EOUT)

execute_process(COMMAND h5diff ${MODULE}-master-mode-master-00.h5
  references/${MODULE}-master-mode-master-00.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (TOUT)
  message(FATAL_ERROR ${TOUT})
endif (TOUT)

#
# Master: The restart mode
#
message(STATUS "Testing restart mode (master)")
execute_process(COMMAND 
  mpirun -np 4 ${MECHANIC} -m master -p ${MODULE} -n ${MODULE}-master-mode -x 10 -y 10 -b 3 -d 13 --test
  --restart-mode --restart-file=${MODULE}-master-mode-master-02.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (EOUT) 
  message(STATUS ${TOUT})
  message(STATUS ${ROUT})
  message(FATAL_ERROR ${EOUT})
endif (EOUT)

execute_process(COMMAND h5diff ${MODULE}-master-mode-master-00.h5
  references/${MODULE}-master-mode-master-00.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (TOUT)
  message(FATAL_ERROR ${TOUT})
endif (TOUT)

