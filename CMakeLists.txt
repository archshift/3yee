CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

SET(EXENAME 3yee)
PROJECT(${EXENAME})

### Sources

SET(SOURCES
    src/main.cpp
    src/shader.cpp
)
INCLUDE_DIRECTORIES(src)

if (${EMSCRIPTEN})
    SET(CMAKE_EXECUTABLE_SUFFIX ".html")
    ADD_COMPILE_OPTIONS("SHELL:-s ALLOW_MEMORY_GROWTH=1 --no-heap-copy")
    ADD_LINK_OPTIONS("SHELL:-s ALLOW_MEMORY_GROWTH=1 --no-heap-copy")
endif()
ADD_COMPILE_OPTIONS(-g -O0 -Wall -Wextra -fno-strict-aliasing)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

### Dependencies

SET(EMSCRIPTEN FALSE CACHE BOOL "Build with emscripten")
if (NOT ${EMSCRIPTEN})
    FIND_PACKAGE(SDL2 REQUIRED)
    FIND_PACKAGE(GLEW 2.0 REQUIRED)
    FIND_PACKAGE(OpenGL REQUIRED)
else()
    ADD_COMPILE_OPTIONS("SHELL:-s USE_SDL=2 -s USE_WEBGL2=1")
    ADD_LINK_OPTIONS("SHELL:-s USE_SDL=2 -s USE_WEBGL2=1")
endif()

ADD_SUBDIRECTORY(stb-image)
INCLUDE_DIRECTORIES(stb-image)

ADD_SUBDIRECTORY(imgui)
INCLUDE_DIRECTORIES(imgui/imgui imgui/imgui/examples imgui/imgui/misc/cpp)

ADD_SUBDIRECTORY(glm)
INCLUDE_DIRECTORIES(glm)

### Executable creation

ADD_EXECUTABLE(${EXENAME} ${SOURCES})
TARGET_LINK_LIBRARIES(${EXENAME}
    stb-image
    imgui
    ${glm_LIBRARY}
)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/vertex.glsl ${CMAKE_CURRENT_BINARY_DIR}/vertex.glsl COPYONLY)
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/fragment.glsl ${CMAKE_CURRENT_BINARY_DIR}/fragment.glsl COPYONLY)

if (NOT ${EMSCRIPTEN})
    TARGET_LINK_LIBRARIES(${EXENAME}
        GLEW::GLEW
        OpenGL::OpenGL
        SDL2::SDL2
    )
else()
    TARGET_LINK_OPTIONS(${EXENAME} PRIVATE "SHELL:--preload-file vertex.glsl --preload-file fragment.glsl")
endif()

