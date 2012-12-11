include_directories(. .. ../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)

add_library (mechanic_module_core SHARED mechanic_module_core.c)
target_link_libraries(mechanic_module_core mpi hdf5 m libmechanic)
install (TARGETS mechanic_module_core DESTINATION lib${LIB_SUFFIX})

