cmake_minimum_required(VERSION 3.5)

#set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g ")

project(armisa)

include_directories("${CMAKE_CURRENT_LIST_DIR}/../../systemc-2.3.2/src")

include_directories("${CMAKE_CURRENT_LIST_DIR}/../include")

include_directories("${CMAKE_CURRENT_LIST_DIR}/../BUS/inc")

include_directories("/usr//local/Cellar/libelf/0.8.13_1/include/libelf")

link_directories("${CMAKE_CURRENT_LIST_DIR}/../../systemc-2.3.2/lib-macosx64")

link_directories("/usr//local/Cellar/libelf/0.8.13_1/lib")

link_directories("${CMAKE_CURRENT_LIST_DIR}/BUS/build")

add_executable(armisa ../Processor/ARMISA.cpp ../Processor/Thumbisa.cpp ../Flash/FlashMem.cpp ../Flash/IntnalMem.cpp ../SRAM/SRAMMem.cpp ../BUS/src/SimpleInitTarg.cpp ../BUS/src/BusTop.cpp ../BUS/src/BusMaster.cpp ../BUS/src/BusSlave.cpp )

target_link_libraries( armisa elf systemc-2.3.2)
