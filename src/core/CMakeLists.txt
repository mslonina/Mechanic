include_directories(. .. ../modes/taskfarm ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src
  ${PROJECT_BINARY_DIR}/src/modes/taskfarm)
set (coresources
    MTypes.h MMechanic2.h
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
    )
set (mainsources
    MMain.c
    )

add_library(core ${coresources})
target_link_libraries(core hdf5 m mpi readconfig)

add_executable(mechanic2 ${mainsources})
target_link_libraries(mechanic2 core taskfarm popt readconfig mpi hdf5 m)
install (TARGETS mechanic2 DESTINATION bin)
install (FILES MMechanic2.h DESTINATION include)
