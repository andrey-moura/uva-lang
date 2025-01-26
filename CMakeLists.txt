#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-lang)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Get the parent directory
get_filename_component(UVALANG_PARENT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)

include("${UVALANG_PARENT_DIRECTORY}/uva/CMakeLists.txt")

uva_library(core)
uva_library(file)
uva_library(console)
uva_library(uva-tests)

include_directories(${CMAKE_CURRENT_LIST_DIR}/include)
include_directories(include/fmt/include)

add_library(uva-lang
    ${CMAKE_CURRENT_LIST_DIR}/src/method.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/preprocessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/extension.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/class.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/interpreter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/object.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/lexer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/parser.cpp
)

add_executable(uva
    ${CMAKE_CURRENT_LIST_DIR}/uva/src/uva.cpp
)

if(UNIX)
    target_link_libraries(uva-lang ${CMAKE_DL_LIBS})
endif()

target_link_libraries(uva uva-lang uva-core uva-console uva-file)

install(TARGETS uva DESTINATION bin)

set(EXTENSIONS_FOLDER "${UVALANG_PARENT_DIRECTORY}/uvalang-extensions")

file(GLOB EXTENSIONS "${EXTENSIONS_FOLDER}/*")

foreach(EXTENSION ${EXTENSIONS})
    if(IS_DIRECTORY ${EXTENSION})
        include("${EXTENSION}/CMakeLists.txt")
    endif()
endforeach()
