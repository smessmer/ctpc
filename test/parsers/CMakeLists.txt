project(parsers-test)

set(SOURCES
	alpha_test.cpp
	alternative_test.cpp
	basic_parsers_test.cpp
	elem_test.cpp
	integer_test.cpp
	map_test.cpp
	match_test.cpp
	opt_test.cpp
	parse_result_test.cpp
    phrase_test.cpp
	rep_test.cpp
	seq_test.cpp
	string_test.cpp
	utils/cvector_test.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})
target_link_libraries(${PROJECT_NAME} googletest parsers)
add_test(${PROJECT_NAME} ${PROJECT_NAME})

target_enable_style_warnings(${PROJECT_NAME})
target_activate_cpp17(${PROJECT_NAME})
