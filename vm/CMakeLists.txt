#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-vm)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(uva-vm
    ${CMAKE_CURRENT_LIST_DIR}/src/vm.cpp
)

target_link_libraries(uva-vm uva-lexer uva-parser uva-extension)