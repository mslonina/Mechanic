include_directories(.)
add_library (mechanic_module_arnoldweb SHARED mechanic_module_arnoldweb.c)
target_link_libraries (mechanic_module_arnoldweb libmechanic m hdf5)
install (TARGETS mechanic_module_arnoldweb DESTINATION lib${LIB_SUFFIX})
