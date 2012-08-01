include_directories(. .. ../modes/taskfarm ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src
  ${PROJECT_BINARY_DIR}/src/modes/taskfarm)
set (coresources
    MTypes.h Mechanic2.h
    MCommon.c MCommon.h
    MBootstrap.c MBootstrap.h
    MCore.c MCore.h
    MModules.c  MModules.h
    MTask.c MTask.h
    MPool.c MPool.h
    MSetup.c MSetup.h
    MMpi.c MMpi.h
    MStorage.c MStorage.h
    MLog.c MLog.h
    MCheckpoint.c MCheckpoint.h
    MRestart.c MRestart.h
    )
set (mainsources
    MMain.c
    )
set (libsources MPublic.c Mechanic2.h)

add_library(libmechanic2 SHARED ${libsources})
set_target_properties(libmechanic2 PROPERTIES OUTPUT_NAME mechanic2)
target_link_libraries(libmechanic2 hdf5 m mpi readconfig)

add_library(core ${coresources})
target_link_libraries(core hdf5 m mpi readconfig libmechanic2)

add_executable(mechanic2 ${mainsources})
target_link_libraries(mechanic2 core taskfarm popt readconfig mpi hdf5 m libmechanic2)

install (TARGETS libmechanic2 DESTINATION lib)
install (TARGETS mechanic2 DESTINATION bin)
install (FILES Mechanic2.h DESTINATION include)
