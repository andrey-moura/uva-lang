#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-lang)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_library(uva-lang
    ${CMAKE_CURRENT_LIST_DIR}/src/class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/method.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/object.cpp

    ${CMAKE_CURRENT_LIST_DIR}/src/false_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/true_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/string_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/integer_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/file_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/std_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/array_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/null_class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/dictionary_class.cpp
)