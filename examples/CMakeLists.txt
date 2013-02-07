install (DIRECTORY ${PROJECT_SOURCE_DIR}/examples DESTINATION share/mechanic 
  PATTERN "CMakeLists.txt" EXCLUDE
  PATTERN "Makefile" EXCLUDE
  PATTERN "*.so" EXCLUDE
  PATTERN "*.o" EXCLUDE
  PATTERN "*.h5" EXCLUDE
  PATTERN "*.dylib" EXCLUDE)
install (FILES ${PROJECT_SOURCE_DIR}/examples/README.md DESTINATION share/mechanic/examples)
