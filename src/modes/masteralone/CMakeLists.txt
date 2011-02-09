include_directories(. .. ../../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)
set (mastersources
    mechanic_mode_masteralone.c
    )

add_library(masteralone ${mastersources})
target_link_libraries(masteralone core)
