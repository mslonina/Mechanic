include_directories(. .. ../../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)

set (
  mastersources
  Master.h
  Master.c
  Worker.c
)

add_library(mechanic_mode_master SHARED ${mastersources})
target_link_libraries(mechanic_mode_master mpi hdf5 libmechanic)
install (TARGETS mechanic_mode_master DESTINATION lib${LIB_SUFFIX})
