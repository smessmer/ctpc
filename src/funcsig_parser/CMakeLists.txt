project(funcsig_parser)

add_library(${PROJECT_NAME} STATIC dummy.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC parsers)
target_activate_cpp17(${PROJECT_NAME})
target_enable_style_warnings(${PROJECT_NAME})

add_executable(${PROJECT_NAME}_bin main.cpp)
target_link_libraries(${PROJECT_NAME}_bin PUBLIC funcsig_parser)
target_activate_cpp17(${PROJECT_NAME}_bin)
target_enable_style_warnings(${PROJECT_NAME}_bin)
