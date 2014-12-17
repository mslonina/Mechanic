set (rngs
  rngs.c mechanic_rngs.h
)

add_library (mechanic_rngs SHARED ${rngs})
install (FILES mechanic_rngs.h DESTINATION include)
install (TARGETS mechanic_rngs DESTINATION lib${LIB_SUFFIX})
