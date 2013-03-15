set (ENV{LD_LIBRARY_PATH} ".")
set (ENV{DYLD_LIBRARY_PATH} ".")

#
# The normal mode
#
message(STATUS "Testing normal mode")
execute_process(COMMAND mpirun -np 4 mechanic -p ${MODULE} -n ${MODULE} -x 10 -y 10 -b 3 -d 13 
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (EOUT) 
  message(STATUS ${TOUT})
  message(STATUS ${ROUT})
  message(FATAL_ERROR ${EOUT})
endif (EOUT)

#execute_process(COMMAND ${CMAKE_COMMAND} -DSOURCEDIR=${CMAKE_CURRENT_SOURCE_DIR} -P
#  ${CMAKE_CURRENT_SOURCE_DIR}/${MODULE}.cmake)
#execute_process(COMMAND diff a.txt r.txt
#  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

execute_process(COMMAND h5diff ${MODULE}-master-00.h5 references/${MODULE}-master-00.h5
  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)

if (TOUT)
  message(FATAL_ERROR ${TOUT})
endif (TOUT)

#
# The restart mode
#
#message(STATUS "Testing restart mode")
#execute_process(COMMAND mpirun -np 4 mechanic -p ${MODULE} -n ${MODULE} -x 10 -y 10 -b 3 -d 13 
#  --restart-mode --restart-file=${MODULE}-master-02.h5
#  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)
#
#if (EOUT) 
#  message(STATUS ${TOUT})
#  message(STATUS ${ROUT})
#  message(FATAL_ERROR ${EOUT})
#endif (EOUT)
#
#execute_process(COMMAND h5diff ${MODULE}-master-00.h5 references/${MODULE}-master-00.h5
#  OUTPUT_VARIABLE TOUT RESULT_VARIABLE ROUT ERROR_VARIABLE EOUT)
#
#if (TOUT)
#  message(FATAL_ERROR ${TOUT})
#endif (TOUT)

