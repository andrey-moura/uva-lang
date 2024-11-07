#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

find_package(SDL2)

if(SDL2_FOUND)
    message(STATUS "SDL2 found")

    project(uva-ui)

    include_directories(${CMAKE_CURRENT_LIST_DIR})

    add_executable(uva-ui
        ${CMAKE_CURRENT_LIST_DIR}/src/ui.cpp
    )

    if(WIN32)
        set_target_properties(target PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
    endif()

    target_link_libraries(uva-ui ${SDL2_LIBRARIES} uva-interpreter uva-lang uva-core uva-console uva-file uva-string)

    include_directories(${SDL2_INCLUDE_DIRS})

    install(TARGETS uva-ui DESTINATION bin)
else()
    message(FATAL_ERROR "SDL2 not found, uva-ui will not be built")
endif()