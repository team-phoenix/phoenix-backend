target_sources( ${PROJECT_NAME}
	PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/audio/audiobuffer.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/audio/audiooutput.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/emulator.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/libretrolibrary.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/logging.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/sharedmemory.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/input/gamepadmanager.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/input/gamepad.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/messageserver.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp


	PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/include/audio/audiobuffer.h
        ${CMAKE_CURRENT_LIST_DIR}/include/audio/audiooutput.h
        ${CMAKE_CURRENT_LIST_DIR}/include/emulator.h
        ${CMAKE_CURRENT_LIST_DIR}/include/libretrolibrary.h
	${CMAKE_CURRENT_LIST_DIR}/include/logging.h
                ${CMAKE_CURRENT_LIST_DIR}/include/input/gamepadmanager.h
        ${CMAKE_CURRENT_LIST_DIR}/include/sharedmemory.h
        ${CMAKE_CURRENT_LIST_DIR}/include/messageserver.h
        ${CMAKE_CURRENT_LIST_DIR}/include/input/gamepad.h
)

target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include" )
target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src" )

target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include/audio" )
target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/audio" )

target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include/input" )
target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src/input" )
