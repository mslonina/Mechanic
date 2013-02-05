include_directories(. .. ../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)

set (rngs
  rngs.c mechanic_rngs.h
)

add_library (mechanic_rngs OBJECT ${rngs})
set_target_properties(mechanic_rngs PROPERTIES COMPILE_FLAGS "-fPIC" )
install (FILES mechanic_rngs.h DESTINATION include)
