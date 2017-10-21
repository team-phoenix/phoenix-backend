include( ${BACKEND_DIR}/cpp/include/tests/CMakeLists.txt )

target_sources( ${PROJECT_NAME}

    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/test_main.cpp

)
