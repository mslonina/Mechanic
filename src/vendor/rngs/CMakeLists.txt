include_directories(. .. ../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)

set (rngs
  rngs.c mechanic_rngs.h
)

add_library (vendor_rngs ${rngs})
target_link_libraries(vendor_rngs m)
install (FILES mechanic_rngs.h DESTINATION include)
