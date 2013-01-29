include_directories(. .. ../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)

set (rngs
  rngs.c rngs.h
)

add_library (vendor_rngs ${rngs})
target_link_libraries(vendor_rngs m)
