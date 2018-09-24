cmake_minimum_required(VERSION 3.5)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")

project(armisa)

include_directories("/usr/local/systemc/include/")

include_directories("{CMAKE_CURRENT_LIST_DIR}")

link_directories("/usr/local/systemc/lib-linux64/")

add_executable(armisa  ARMISA_M0p.cpp )

target_link_libraries( armisa systemc )
