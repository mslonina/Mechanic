include_directories(. .. ../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src
  ../vendor/rngs
)

add_library (mechanic_module_core SHARED mechanic_module_core.c)
target_link_libraries(mechanic_module_core mpi hdf5 libmechanic)
install (TARGETS mechanic_module_core DESTINATION lib${LIB_SUFFIX})

