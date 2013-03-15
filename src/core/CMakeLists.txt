# http://stackoverflow.com/questions/11429055/cmake-how-create-a-single-shared-library-from-all-static-libraries-of-subprojec
include_directories(. .. ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src
  ${PROJECT_BINARY_DIR}/src/vendor
  ${PROJECT_BINARY_DIR}/src/vendor/rngs
  ../vendor/rngs
  )

set (
  mainsources
  M2Main.c
  M2Core.c
)

set (
  libsources
  M2Apublic.c
  M2Cpublic.c
  M2Epublic.c
  M2Hpublic.c
  M2Mpublic.c
  M2Ppublic.c
  M2Rpublic.c
  M2Spublic.c
  M2Tpublic.c
  M2Wpublic.c
)

set (
  coresources
  M2Aprivate.c
  M2Bprivate.c
  M2Cprivate.c
  M2CSprivate.c
  M2Pprivate.c
  M2Rprivate.c
  M2Sprivate.c
  M2Wprivate.c
)

set (
  headers
  mechanic.h
  M2Main.h
  M2Apublic.h
  M2Cpublic.h
  M2Epublic.h
  M2Hpublic.h
  M2Mpublic.h
  M2Ppublic.h
  M2Rpublic.h
  M2Spublic.h
  M2Tpublic.h
  M2Wpublic.h
)

#add_library(libmechanic SHARED ${libsources} $<TARGET_OBJECTS:mechanic_rngs>)
add_library(libmechanic SHARED ${libsources})
set_target_properties(libmechanic PROPERTIES OUTPUT_NAME mechanic)
target_link_libraries(libmechanic hdf5 mpi)

add_library(core ${coresources})
target_link_libraries(core hdf5 mpi m libmechanic)

add_executable(mechanic ${mainsources})
target_link_libraries(mechanic core popt mpi hdf5 libmechanic)

install (TARGETS libmechanic DESTINATION lib)
install (TARGETS mechanic DESTINATION bin)
install (FILES ${headers} DESTINATION include)
