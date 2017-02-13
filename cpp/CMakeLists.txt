target_sources( ${PROJECT_NAME}
	PRIVATE
	${CMAKE_CURRENT_LIST_DIR}/src/audiobuffer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/audiooutput.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/coredemuxer.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/coresymbols.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/logging.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/serialization.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/sharedprocessmemory.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/inputmanager.cpp
	${CMAKE_CURRENT_LIST_DIR}/src/main.cpp


	PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/include/audiobuffer.h
	${CMAKE_CURRENT_LIST_DIR}/include/audiooutput.h
	${CMAKE_CURRENT_LIST_DIR}/include/coredemuxer.h
	${CMAKE_CURRENT_LIST_DIR}/include/coresymbols.h
	${CMAKE_CURRENT_LIST_DIR}/include/logging.h
	${CMAKE_CURRENT_LIST_DIR}/include/serialization.h
        ${CMAKE_CURRENT_LIST_DIR}/include/inputmanager.h

	${CMAKE_CURRENT_LIST_DIR}/include/sharedprocessmemory.h
)

target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include" )
target_include_directories( ${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_LIST_DIR}/src" )
