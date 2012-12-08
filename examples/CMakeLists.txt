install (DIRECTORY ${PROJECT_SOURCE_DIR}/examples DESTINATION doc/mechanic FILES_MATCHING PATTERN "*.c")
install (FILES ${PROJECT_SOURCE_DIR}/examples/README.md DESTINATION doc/mechanic/examples)
install (FILES ${PROJECT_SOURCE_DIR}/examples/Makefile.example DESTINATION doc/mechanic/examples)

