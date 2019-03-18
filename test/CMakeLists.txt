if (BUILD_TESTING)
  include_directories(../src)

  add_subdirectory(parsers)
  add_subdirectory(funcsig_parser)
endif(BUILD_TESTING)
