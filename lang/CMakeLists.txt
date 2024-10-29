#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-lang)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(uva-lang
    ${CMAKE_CURRENT_LIST_DIR}/src/object.cpp
)