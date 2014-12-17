set (
  module_core_src
  mechanic_module_core.c
)

add_library (mechanic_module_core SHARED ${module_core_src})
target_link_libraries(mechanic_module_core mpi hdf5 libmechanic)
install (TARGETS mechanic_module_core DESTINATION lib${LIB_SUFFIX})

