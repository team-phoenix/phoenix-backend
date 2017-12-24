cmake_minimum_required( VERSION 3.6.0 FATAL_ERROR )

project (phoenix)

add_subdirectory (backend)

enable_testing ()
add_subdirectory (unit_tests)
