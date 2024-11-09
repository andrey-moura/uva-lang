#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-ui)

include_directories(${CMAKE_CURRENT_LIST_DIR})

if(WIN32)
    set(GUI_BACKEND "win32")
elseif(LINUX)
    set(GUI_BACKEND "gtk3")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

    include_directories(${GTK3_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

add_executable(uva-ui
    ${CMAKE_CURRENT_LIST_DIR}/src/${GUI_BACKEND}/app.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/${GUI_BACKEND}/frame.cpp
    
    ${CMAKE_CURRENT_LIST_DIR}/src/theme.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ui.cpp
)

if(WIN32)
    set_target_properties(target PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
elseif(LINUX)
    target_link_libraries(uva-ui ${GTK3_LIBRARIES})
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

include_directories(${CMAKE_SOURCE_DIR}/lib/external/SDL-preview-3.1.6/include)

target_link_libraries(uva-ui uva-interpreter uva-lang uva-core uva-console uva-file uva-string)

install(TARGETS uva-ui DESTINATION bin)