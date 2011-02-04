include_directories(. .. ../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)
add_library (mechanic_module_module SHARED mechanic_module_module.c)
target_link_libraries (mechanic_module_module libmechanic)
install (TARGETS mechanic_module_module DESTINATION lib)
