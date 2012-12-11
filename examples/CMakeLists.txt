install (DIRECTORY ${PROJECT_SOURCE_DIR}/examples DESTINATION share/mechanic PATTERN "CMakeLists.txt" EXCLUDE)
install (FILES ${PROJECT_SOURCE_DIR}/examples/README.md DESTINATION share/mechanic/examples)
