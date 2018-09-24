cmake_minimum_required(VERSION 3.5)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")

project(ProcessorBus)

include_directories("/usr/local/systemc/include/")

include_directories("${CMAKE_CURRENT_LIST_DIR}/inc")

link_directories("/usr/local/systemc/lib-linux64/")

add_library(ProcessorBus ${CMAKE_CURRENT_LIST_DIR}/src/SimpleInitTarg.cpp ${CMAKE_CURRENT_LIST_DIR}/src/BusTop.cpp ${CMAKE_CURRENT_LIST_DIR}/src/BusMaster.cpp ${CMAKE_CURRENT_LIST_DIR}/src/BusSlave.cpp )

target_link_libraries( ProcessorBus systemc )

#include(${CMAKE_CURRENT_LIST_DIR}/examples/Blocking/CMakeLists.txt)

#include(${CMAKE_CURRENT_LIST_DIR}/examples/NonBlocking/CMakeLists.txt)
