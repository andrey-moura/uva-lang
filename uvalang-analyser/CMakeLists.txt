#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uvalang-analyser)

include_directories(${CMAKE_CURRENT_LIST_DIR})

add_executable(uvalang-analyser
    ${CMAKE_CURRENT_LIST_DIR}/src/analyser.cpp
)

target_link_libraries(uvalang-analyser uva-extension uva-preprocessor uva-interpreter uva-lang uva-core uva-console uva-file uva-string ${wxWidgets_LIBRARIES})

install(TARGETS uvalang-analyser DESTINATION bin)