project(parsers)

add_library(${PROJECT_NAME} STATIC dummy.cpp)

target_activate_cpp17(${PROJECT_NAME})
target_enable_style_warnings(${PROJECT_NAME})
