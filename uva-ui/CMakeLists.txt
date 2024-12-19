#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-ui)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(UVA_UI_BACKEND "NONE")

if(WIN32)
    set(UVA_UI_BACKEND "WIN32")
elseif(LINUX)
    find_package(PkgConfig)

    if(PKG_CONFIG_FOUND)
        pkg_check_modules(GTK3 gtk+-3.0)

        if(GTK3_FOUND)
            set(UVA_UI_BACKEND "GTK3")
        endif()

        include_directories(${GTK3_INCLUDE_DIRS})
    endif()
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

if (NOT UVA_UI_BACKEND STREQUAL "NONE")
    # Add uvaui as shared library

    add_library(uvaui-shared SHARED
        ${CMAKE_CURRENT_LIST_DIR}/src/ui.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/app_class.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/frame_class.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/${UVA_UI_BACKEND}/app.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/${UVA_UI_BACKEND}/frame.cpp

    )

    include_directories(${CMAKE_CURRENT_LIST_DIR}/include)

    if(WIN32)
        set_target_properties(uvaui-shared PROPERTIES WIN32_EXECUTABLE true)
    elseif(LINUX)
        target_link_libraries(uvaui-shared ${GTK3_LIBRARIES})
    else()
        message(FATAL_ERROR "Unsupported platform")
    endif()

    #target_link_libraries(uvaui-shared  uva-preprocessor uva-interpreter uva-lang uva-core uva-console uva-file )
    target_link_libraries(uvaui-shared uva-lang uva-extension uva-console uva-lexer uva-interpreter)

    install(TARGETS uvaui-shared DESTINATION bin)

    add_definitions(-D"__UVA_UI_BACKEND_${UVA_UI_BACKEND}__")
endif()

message(STATUS "UVA_UI_BACKEND: ${UVA_UI_BACKEND}")