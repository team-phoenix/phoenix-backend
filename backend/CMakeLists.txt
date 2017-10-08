cmake_minimum_required( VERSION 3.6.0 FATAL_ERROR )

list( APPEND CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_LIST_DIR}/cmake
        ${CMAKE_CURRENT_LIST_DIR}/externals/cmake-modules
)

project( backend )

add_executable( ${PROJECT_NAME} "" )

##
## Source files
##

include( ${CMAKE_CURRENT_LIST_DIR}/cpp/CMakeLists.txt )
include( ${CMAKE_CURRENT_LIST_DIR}/externals/CMakeLists.txt )

target_sources( ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/cpp/src/main.cpp
)

target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}" )

##
## Compiler
##

set_target_properties( ${PROJECT_NAME}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

set_target_properties( ${PROJECT_NAME} PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED 14 )

##
## Distribution
##

##
## Distribution
##

#include( cmake/DistCommon.cmake )
