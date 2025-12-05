cmake_minimum_required(VERSION 3.10.0)
project(shenanigans VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED)

add_executable(shenanigans 
    src/main.cpp
    src/scripting.c
    external/glad/src/glad.c
)

include_directories(external/glm)

target_include_directories(shenanigans PUBLIC
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/sdl3/include
    "${CMAKE_SOURCE_DIR}/external/lua/src"
)

file(GLOB LUA_SRC "${CMAKE_SOURCE_DIR}/external/lua/src/*.c")
add_library(lua STATIC ${LUA_SRC})

target_link_options(shenanigans PRIVATE 
    "-L${CMAKE_SOURCE_DIR}/external/sdl3"
)

target_link_libraries(shenanigans PUBLIC
    OpenGL::GL
    SDL3
    lua
)

# copies SDL3.dll to exe directory
add_custom_command(TARGET shenanigans POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/external/sdl3/SDL3.dll"
        $<TARGET_FILE_DIR:shenanigans>
    COMMENT "Copying SDL3.dll to output directory"
)
