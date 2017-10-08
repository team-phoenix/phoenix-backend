
project( testcases )

list( APPEND CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_SOURCE_DIR}/../backend/cmake
        ${CMAKE_CURRENT_SOURCE_DIR}/../backend/externals/cmake-modules
)

add_executable( ${PROJECT_NAME} "" )

# Externals
include( ${CMAKE_CURRENT_LIST_DIR}/../backend/externals/CMakeLists.txt)

# Sources
include( ${CMAKE_CURRENT_LIST_DIR}/cpp/CMakeLists.txt )

add_test (NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} )
