include_directories(. .. ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src/fortran)
add_library(mechanic_fortran SHARED mechanic_fortran.F90)
target_link_libraries(mechanic_fortran libmechanic hdf5_fortran hdf5 m mpi)

install (TARGETS mechanic_fortran DESTINATION lib${LIB_SUFFIX})
install (FILES ${PROJECT_BINARY_DIR}/src/fortran/mechanic_fortran.mod DESTINATION include)
