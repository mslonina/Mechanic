install (DIRECTORY ${PROJECT_SOURCE_DIR}/examples DESTINATION share/mechanic FILES_MATCHING PATTERN "*.c")
install (FILES ${PROJECT_SOURCE_DIR}/examples/README.md DESTINATION share/mechanic/examples)
install (FILES ${PROJECT_SOURCE_DIR}/examples/Makefile.example DESTINATION share/mechanic/examples)

