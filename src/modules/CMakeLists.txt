include_directories(. .. ../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)

add_library (mechanic_module_core SHARED mechanic_module_core.c)
install (TARGETS mechanic_module_core DESTINATION lib${LIB_SUFFIX})

add_library (mechanic_module_test SHARED mechanic_module_test.c)
install (TARGETS mechanic_module_test DESTINATION lib${LIB_SUFFIX})
