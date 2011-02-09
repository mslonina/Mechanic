include_directories(. core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)
add_executable(mechanic mechanic.c)
target_link_libraries(mechanic core farm masteralone popt readconfig mpi hdf5 m)
install (TARGETS mechanic DESTINATION bin)
