include_directories(. .. ../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)
#enable_language(Fortran)

add_library (mechanic_module_core SHARED mechanic_module_core.c)
target_link_libraries(mechanic_module_core readconfig mpi hdf5 m)
install (TARGETS mechanic_module_core DESTINATION lib${LIB_SUFFIX})

#add_library (mechanic_module_test SHARED mechanic_module_test.c fortran.F90)
#target_link_libraries(mechanic_module_test readconfig mpi hdf5 m)
#install (TARGETS mechanic_module_test DESTINATION lib${LIB_SUFFIX})

add_library (mechanic_module_web SHARED mechanic_module_web.c mechanic_module_web_lib.c)
target_link_libraries(mechanic_module_web readconfig mpi hdf5 m)
install (TARGETS mechanic_module_web DESTINATION lib${LIB_SUFFIX})

add_library (mechanic_module_aweb SHARED mechanic_module_aweb.c mechanic_module_web_lib.c)
target_link_libraries(mechanic_module_aweb readconfig mpi hdf5 m)
install (TARGETS mechanic_module_aweb DESTINATION lib${LIB_SUFFIX})

add_library (mechanic_module_ga SHARED mechanic_module_ga.c)
target_link_libraries(mechanic_module_ga readconfig mpi hdf5 m)
install (TARGETS mechanic_module_ga DESTINATION lib${LIB_SUFFIX})
