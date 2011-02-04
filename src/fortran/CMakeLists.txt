include_directories(.)
add_library(mechanic_fortran SHARED mechanic_fortran.F90)
target_link_libraries(mechanic_fortran mechanic_t hdf5 m mpi)
install (FILES mechanic_fortran.mod DESTINATION include)
