cmake_minimum_required(VERSION 3.10)

project(CppCodeOpt)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_STANDARD 17)

add_executable(1_enum 1_enum.cpp)

add_executable(2_pass_by_value_and_move 2_pass_by_value_and_move.cpp)

add_executable(3_float_vs_double 3_float_vs_double.cpp)

find_package(OpenCV REQUIRED)
add_executable(4_pipelining 4_pipelining.cpp)
target_link_libraries(4_pipelining ${OpenCV_LIBS})
