cmake_minimum_required(VERSION 3.2)

set(Target ThreadTest)

find_package( Threads )

add_executable(${Target} test.cpp)

target_link_libraries (${Target} ${CMAKE_THREAD_LIBS_INIT}) 