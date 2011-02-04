include_directories(. .. ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)
set (coresources
    mechanic.h
    mechanic_internals.h
    mechanic_core_checkpoint.c
    mechanic_core_derived.c
    mechanic_core_hdf5.c
    mechanic_core_setup.c
    )
set (sharedsources
    mechanic_core_internals.c
    mechanic_core_devel.c
    mechanic_core_errors.c
    mechanic_core_tools.c
    )

add_library(core ${coresources} ${sharedsources})
target_link_libraries(core hdf5 m mpi readconfig)

add_library(mechanic_t SHARED ${sharedsources})
target_link_libraries(mechanic_t hdf5 m mpi readconfig)

install (TARGETS mechanic_t DESTINATION lib)
install (FILES mechanic.h DESTINATION include)
install (FILES mechanic_internals.h DESTINATION include)
