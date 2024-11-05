#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva)

include(include/uva/CMakeLists.txt)

uva_library(string)
uva_library(core)
uva_library(json)
uva_library(file)
uva_library(console)

include_directories(${CMAKE_CURRENT_LIST_DIR})
include_directories(include/fmt/include)

add_library(uva-parser
    parser/parser.cpp
)

target_link_libraries(uva-parser uva-lexer)

add_library(uva-extension
    extension/src/extension.cpp
)

if(DEFINED LANGUANIZER_ENABLED)
	include("languanizer/CMakeLists.txt")
endif()

include("interpreter/CMakeLists.txt")
include("lexer/CMakeLists.txt")
include("lang/CMakeLists.txt")
include("uva-ui/CMakeLists.txt")

if(DEFINED WX_ENABLED)
	include("uva-wx/CMakeLists.txt")
endif()

add_executable(uva
    uva/src/uva.cpp
    uva/src/main.cpp
    uva/src/winmain.cpp
)

target_link_libraries(uva uva-interpreter uva-ui uva-lang uva-core uva-console uva-file uva-string ${wxWidgets_LIBRARIES})

if(UNIX AND NOT APPLE)
    target_link_options(uva PRIVATE "-rdynamic")
    target_link_libraries(uva dl)
elseif(WIN32)
    set_target_properties(uva PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

install(TARGETS uva DESTINATION bin)