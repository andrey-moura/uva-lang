#Require a minimum version
cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)

project(uva-languanizer)

if(FALSE)
    # sudo apt install libclang-dev
    find_package(Clang)
endif()

if(DEFINED CLANG_INCLUDE_DIRS)
    include_directories(${CLANG_INCLUDE_DIRS})
    add_definitions(${CLANG_DEFINITIONS})
    add_definitions("-lclang")

    add_executable(uva-languanizer
        ${CMAKE_CURRENT_LIST_DIR}/src/languanizer.cpp
    )

    target_link_libraries(uva-languanizer uva-console uva-string /usr/lib/llvm-10/lib/libclang.so)
else()
    message("Clang not found, skipping languanizer")
endif()