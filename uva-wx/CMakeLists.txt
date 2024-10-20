#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-wx)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(uva-wx-extension
    ${CMAKE_CURRENT_LIST_DIR}/src/wx-extension.cpp
)

add_executable(uva-wx
    ${CMAKE_CURRENT_LIST_DIR}/src/main.cpp
)