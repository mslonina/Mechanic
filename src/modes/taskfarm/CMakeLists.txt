set (
  farmsources
  Taskfarm.h
  Master.c
  Worker.c
)

add_library(mechanic_mode_taskfarm SHARED ${farmsources})
target_link_libraries(mechanic_mode_taskfarm mpi hdf5 libmechanic)
install (TARGETS mechanic_mode_taskfarm DESTINATION lib${LIB_SUFFIX})
