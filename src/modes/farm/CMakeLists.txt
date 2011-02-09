include_directories(. .. ../../core ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}/src)
set (farmsources
    mechanic_mode_farm.c
    mechanic_mode_farm_master.c
    mechanic_mode_farm_slave.c
    )

add_library(farm ${farmsources})
target_link_libraries(farm core)
