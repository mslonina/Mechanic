include_directories(. .. ../../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)
set (farmsources
    Taskfarm.c
    Master.c
    Worker.c
    )

add_library(taskfarm ${farmsources})
target_link_libraries(taskfarm core)
