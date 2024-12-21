#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-lang)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include("../uva/CMakeLists.txt")

uva_library(core)
uva_library(file)
uva_library(console)
# uva_library(binary)
# uva_library(uva-tests)
# uva_library(xml)
# uva_library(uva-renderer)

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(include/fmt/include)

add_library(uva-parser
    parser/parser.cpp
)

target_link_libraries(uva-parser uva-lexer)

add_library(uva-extension
    extension/src/extension.cpp
)

include("interpreter/CMakeLists.txt")
include("lexer/CMakeLists.txt")
include("lang/CMakeLists.txt")
include("preprocessor/CMakeLists.txt")

add_executable(uva
    uva/src/uva.cpp
)

if(UNIX)
    target_link_libraries(uva-extension ${CMAKE_DL_LIBS})
endif()

target_link_libraries(uva uva-extension uva-preprocessor uva-interpreter uva-lang uva-core uva-console uva-file)

install(TARGETS uva DESTINATION bin)