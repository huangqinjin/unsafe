project(unsafe_test)

file(DOWNLOAD
    https://github.com/catchorg/Catch2/releases/download/v2.13.10/catch.hpp
    ${CMAKE_CURRENT_LIST_DIR}/catch.hpp
    EXPECTED_HASH SHA512=b7dd8acce2d32e86f5356c7b7e96a72c6aecb0529af29a7ab85b8dfb1649d510bcfe117f57691b75783ca90fd21c347f64c9cf6d4b996d686f82f081840e89cb
)

file(GLOB SRC_FILES *.cpp)
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE unsafe)

add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
