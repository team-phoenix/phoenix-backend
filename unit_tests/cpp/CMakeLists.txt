target_sources( ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/test_main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/mock_emulator.cpp

    PUBLIC
    ${BACKEND_DIR}/cpp/include/emulator.h

)
