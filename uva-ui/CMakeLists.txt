#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-ui)

include_directories(${CMAKE_CURRENT_LIST_DIR})

set(GUI_BACKEND "NONE")

if(WIN32)
    set(GUI_BACKEND "win32")
elseif(LINUX)
    find_package(PkgConfig)

    if(PKG_CONFIG_FOUND)
        pkg_check_modules(GTK3 gtk+-3.0)

        if(GTK3_FOUND)
            set(GUI_BACKEND "gtk3")
        endif()

        include_directories(${GTK3_INCLUDE_DIRS})
    endif()
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

if (NOT GUI_BACKEND STREQUAL "NONE")
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

    target_link_libraries(uva-ui uva-interpreter uva-lang uva-core uva-console uva-file uva-string)

    install(TARGETS uva-ui DESTINATION bin)
endif()

message(STATUS "GUI_BACKEND: ${GUI_BACKEND}")