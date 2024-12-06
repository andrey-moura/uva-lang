project(uva-widgets)

add_library(uva-widgets
    ${CMAKE_CURRENT_LIST_DIR}/layout.cpp
)

add_executable(uva-renderer
    ${CMAKE_CURRENT_LIST_DIR}/main.cpp
)

target_link_libraries(uva-widgets -lSDL2 uva-xml uva-binary)
target_link_libraries(uva-renderer -lSDL2main uva-widgets)
target_link_libraries(uva-renderer -lSDL2_ttf -lSDL2_gfx)
target_link_libraries(uva-renderer uva-drawing ${GTK3_LIBRARIES})

uva_tests_configure(uva-widgets)

include_directories(${CMAKE_CURRENT_LIST_DIR}/)