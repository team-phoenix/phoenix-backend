cmake_minimum_required( VERSION 3.6.0 FATAL_ERROR )
list( APPEND CMAKE_MODULE_PATH
        ${CMAKE_CURRENT_LIST_DIR}/cmake
        ${CMAKE_CURRENT_LIST_DIR}/externals/cmake-modules
)

project( SandboxedBackend )
add_executable( ${PROJECT_NAME} "" )


##
## Source files
##

include( ${CMAKE_CURRENT_LIST_DIR}/cpp/CMakeLists.txt )

target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}" )

##
## Externals
##

find_package( Libsamplerate REQUIRED )
if( LIBSAMPLERATE_FOUND )
        include_directories( ${LIBSAMPLERATE_INCLUDE_DIR} )
        target_link_libraries( ${PROJECT_NAME} ${LIBSAMPLERATE_LIBRARY} )
endif()

find_package( SDL2 REQUIRED )
if( SDL2_FOUND )
        include_directories( ${SDL2_INCLUDE_DIR} )
        target_link_libraries( ${PROJECT_NAME} ${SDL2_LIBRARY} ${SDL2_SDLMAIN_LIBRARY} )
endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed and rcc.
set_target_properties( ${PROJECT_NAME} PROPERTIES AUTOMOC ON AUTORCC ON )


find_package( Qt5 REQUIRED COMPONENTS  Core Multimedia Gui )
target_link_libraries( ${PROJECT_NAME} Qt5::Core Qt5::Multimedia Qt5::Gui )

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
