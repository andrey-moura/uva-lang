#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-lexer)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(uva-lexer
    ${CMAKE_CURRENT_LIST_DIR}/src/lexer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/cursor.cpp
)