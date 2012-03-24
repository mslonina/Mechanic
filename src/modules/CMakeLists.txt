include_directories(. .. ../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)

add_library (mechanic_module_core SHARED mechanic_module_core.c)
target_link_libraries(mechanic_module_core readconfig mpi hdf5 m)
install (TARGETS mechanic_module_core DESTINATION lib${LIB_SUFFIX})

add_library (mechanic_module_test SHARED mechanic_module_test.c)
target_link_libraries(mechanic_module_test readconfig mpi hdf5 m)
install (TARGETS mechanic_module_test DESTINATION lib${LIB_SUFFIX})

add_library (mechanic_module_web SHARED mechanic_module_web.c mechanic_module_web_lib.c)
target_link_libraries(mechanic_module_web readconfig mpi hdf5 m)
install (TARGETS mechanic_module_web DESTINATION lib${LIB_SUFFIX})
