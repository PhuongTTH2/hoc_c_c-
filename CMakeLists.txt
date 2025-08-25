cmake_minimum_required(VERSION 3.10)
project(SampleApp)

set(CMAKE_CXX_STANDARD 11)

add_library(math_utils math_utils.cpp)
add_executable(app main.cpp)
target_link_libraries(app math_utils)

add_executable(test math_utils.test.cpp)
target_link_libraries(test math_utils)
target_include_directories(test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})