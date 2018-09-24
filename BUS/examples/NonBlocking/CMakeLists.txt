cmake_minimum_required(VERSION 3.5)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")

project(ProcessorBus_example)

include_directories("/usr/local/systemc/include/")

include_directories("${CMAKE_CURRENT_LIST_DIR}/../../inc/")

link_directories("/usr/local/systemc/lib-linux64/")

link_directories("${CMAKE_CURRENT_LIST_DIR}/../../build/")

add_executable(ProcessorBus_example  ProcessorBus_example.cpp )

target_link_libraries( ProcessorBus_example systemc ProcessorBus)