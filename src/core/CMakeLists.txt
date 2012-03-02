include_directories(. .. ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)
set (coresources
    MTypes.h MMechanic2.h
    MBootstrap.c
    MCore.c
    MModules.c
    MTask.c
    MPool.c
    MSetup.c
    MMpi.c
    MStorage.c
    MLog.c
    MTools.c
    )
set (mainsources
    MMain.c
    )

add_library(core ${coresources})
target_link_libraries(core hdf5 m mpi readconfig ltdl)

add_executable(mechanic2 ${mainsources})
target_link_libraries(mechanic2 core popt readconfig mpi hdf5 m ltdl)
install (TARGETS mechanic2 DESTINATION bin)
install (FILES MMechanic2.h DESTINATION include)
