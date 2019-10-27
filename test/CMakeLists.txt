cmake_minimum_required(VERSION 2.8)
project(Test)

set(CMAKE_CXX_FLAGS "-std=c++14")
set(CMAKE_BUILD_TYPE "Debug")

include_directories(../include)
link_directories(../lib)

add_executable(Test test.cpp)
target_link_libraries(Test libpipe.dylib)