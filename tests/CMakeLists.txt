include_directories(. .. ${PROJECT_BINARY_DIR} 
  ${PROJECT_BINARY_DIR}/src
  ${PROJECT_BINARY_DIR}/src/core
  ${PROJECT_BINARY_DIR}/examples
  ${PROJECT_BINARY_DIR}/examples/c
  ../examples/c
  ../src/core
  )
enable_testing()

#file(GLOB modules "../examples/c/*.c")
set (
  modules
  ex_map
  ex_mandelbrot
  ex_setup
  ex_poolsetup
  ex_prepareprocess
  ex_createpool
  ex_changepoollayout
  ex_changepoollayout2
  ex_datatypes
  ex_dim
  ex_attr
  ex_reset
  ex_chreset
  ex_pool
  ex_poolsize
  ex_poolmask
  ex_dset
  ex_node
  ex_loop
  ex_readfile
  ex_readfile_setup
  ex_ga
  ex_rngs
  ex_taskcheckpoint
  ex_stage
)

configure_file(../examples/c/readfile.txt readfile.txt COPYONLY)

foreach(module ${modules})
  add_library(mechanic_module_t${module} SHARED ../examples/c/mechanic_module_${module}.c)
  target_link_libraries(mechanic_module_t${module} mpi hdf5 hdf5_hl libmechanic)
endforeach()

foreach(module ${modules})
  set (test mpirun -np 4 mechanic -p t${module} -n t${module})
  add_test(NAME ${module} COMMAND mpirun -np 4 mechanic -p t${module} -x 10 -y 10 -n t${module})
  #message(${test})
  #  add_test(NAME ${module} COMMAND ${CMAKE_COMMAND} -DTEST=${test}
  #  -DSOURCEDIR=${CMAKE_CURRENT_SOURCE_DIR} -P ${CMAKE_CURRENT_SOURCE_DIR}/test.cmake)
endforeach()
