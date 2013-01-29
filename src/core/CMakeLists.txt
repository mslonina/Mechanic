include_directories(. .. ../modes/taskfarm ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src
  ${PROJECT_BINARY_DIR}/src/modes/taskfarm
  ${PROJECT_BINARY_DIR}/src/vendor
  ${PROJECT_BINARY_DIR}/src/vendor/rngs
  ../vendor/rngs
  )

set (
  coresources
  mechanic.h
  MTypes.c MTypes.h
  MCommon.c MCommon.h
  MBootstrap.c MBootstrap.h
  MCore.c MCore.h
  MModules.c  MModules.h
  MTask.c MTask.h
  MPool.c MPool.h
  MConfig.c MConfig.h
  MSetup.c MSetup.h
  MMpi.c MMpi.h
  MStorage.c MStorage.h
  MLog.c MLog.h
  MWork.c MWork.h
  MCheckpoint.c MCheckpoint.h
  MRestart.c MRestart.h
)

set (
  mainsources
  MMain.c
)

set (
  libsources
  MConfig.c MConfig.h
  MPublic.c mechanic.h
)

add_library(libmechanic SHARED ${libsources} $<TARGET_OBJECTS:mechanic_rngs>)
set_target_properties(libmechanic PROPERTIES OUTPUT_NAME mechanic)
target_link_libraries(libmechanic hdf5 hdf5_hl m mpi)

add_library(core ${coresources})
target_link_libraries(core hdf5 hdf5_hl m mpi libmechanic)

add_executable(mechanic ${mainsources})
target_link_libraries(mechanic core taskfarm popt mpi hdf5 hdf5_hl m libmechanic)

install (TARGETS libmechanic DESTINATION lib)
install (TARGETS mechanic DESTINATION bin)
install (FILES mechanic.h DESTINATION include)
