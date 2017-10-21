
project( unit_tests )

set( BACKEND_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../backend )

list( APPEND CMAKE_MODULE_PATH
        ${BACKEND_DIR}/cmake
        ${BACKEND_DIR}/externals/cmake-modules
)


add_executable( ${PROJECT_NAME} "" )

# Externals
include( ${BACKEND_DIR}/externals/CMakeLists.txt)

# Sources
include( ${CMAKE_CURRENT_LIST_DIR}/cpp/CMakeLists.txt )

add_test (NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} )
