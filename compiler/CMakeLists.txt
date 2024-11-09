#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-compiler)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(uvalang
    ${CMAKE_CURRENT_LIST_DIR}/src/compiler.cpp
)

target_link_libraries(uvalang uva-lexer uva-parser uva-extension uva-console)