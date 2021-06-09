project(data_structure)
cmake_minimum_required(VERSION 3.18)
enable_language(CXX)

set(CMAKE_CXX_STANDARD 20)

add_library(skiplist INTERFACE skip_list.h)
add_executable(skip_list_test skip_list_test.cpp)
target_link_libraries(skip_list_test skiplist)