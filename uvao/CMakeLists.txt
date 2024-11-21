#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uvao)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(uvao
    ${CMAKE_CURRENT_LIST_DIR}/src/uvao.cpp
)

target_link_libraries(uvao uva-extension uva-preprocessor uva-interpreter uva-lang uva-core uva-console uva-file uva-string ${wxWidgets_LIBRARIES})