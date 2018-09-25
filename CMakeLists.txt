cmake_minimum_required(VERSION 3.5)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")

project(armisa)

include_directories("/usr/local/systemc/include/")

include_directories("${CMAKE_CURRENT_LIST_DIR}/include")

link_directories("/usr/local/systemc/lib-linux64/")

link_directories("${CMAKE_CURRENT_LIST_DIR}/BUS/build")

add_executable(armisa  Processor/ARMISA.cpp Processor/Thumbisa.cpp Flash/FlashMem.cpp SRAM/SRAMMem.cpp)

target_link_libraries( armisa systemc ProcessorBus)
