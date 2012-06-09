include_directories(. .. ../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)

add_library (mechanic_module_core SHARED mechanic_module_core.c)
target_link_libraries(mechanic_module_core readconfig mpi hdf5 m libmechanic2)
install (TARGETS mechanic_module_core DESTINATION lib${LIB_SUFFIX})

add_library (mechanic_module_aweb SHARED mechanic_module_aweb.c mechanic_module_aweb_lib.c)
target_link_libraries(mechanic_module_aweb readconfig mpi hdf5 m libmechanic2)
install (TARGETS mechanic_module_aweb DESTINATION lib${LIB_SUFFIX})

add_library (mechanic_module_ga SHARED mechanic_module_ga.c)
target_link_libraries(mechanic_module_ga readconfig mpi hdf5 m libmechanic2)
install (TARGETS mechanic_module_ga DESTINATION lib${LIB_SUFFIX})
