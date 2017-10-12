include_directories( ${CMAKE_CURRENT_LIST_DIR}/../cpp/include)
include_directories( ${CMAKE_CURRENT_LIST_DIR}/../cpp/include/input )
include_directories( ${CMAKE_CURRENT_LIST_DIR}/../cpp/include/audio )

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


find_package( Qt5 REQUIRED COMPONENTS Core Multimedia Gui Test )
target_link_libraries( ${PROJECT_NAME} Qt5::Core Qt5::Multimedia Qt5::Gui Qt5::Test )
