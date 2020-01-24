INCLUDE_DIRECTORIES(imgui)

ADD_DEFINITIONS(-DIMGUI_IMPL_OPENGL_LOADER_GLEW)

ADD_LIBRARY(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_widgets.cpp

    imgui/misc/cpp/imgui_stdlib.cpp

    imgui/examples/imgui_impl_opengl3.cpp
    imgui/examples/imgui_impl_sdl.cpp
)

if (NOT ${EMSCRIPTEN})
    TARGET_LINK_LIBRARIES(imgui
        SDL2::SDL2
    )
endif()
