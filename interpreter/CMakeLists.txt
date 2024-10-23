#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-interpreter)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(uva-interpreter
    ${CMAKE_CURRENT_LIST_DIR}/src/interpreter.cpp
)

target_link_libraries(uva-interpreter uva-lexer uva-parser uva-extension)