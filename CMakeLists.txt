cmake_minimum_required(VERSION 3.10.0)
project(shenanigans VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED)

add_executable(shenanigans 
    src/main.cpp
    src/scripting.c
    src/level.cpp

    src/graphics/camera.cpp
    src/graphics/mesh.cpp
    src/graphics/shaders.cpp
    src/graphics/texture.cpp
    src/graphics/model.cpp
    src/graphics/shapes.cpp
)

target_include_directories(shenanigans PUBLIC
    ${CMAKE_SOURCE_DIR}/external/include
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/sdl3/include
    ${CMAKE_SOURCE_DIR}/external/lua/src

    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/graphics
)

file(GLOB LUA_SRC "${CMAKE_SOURCE_DIR}/external/lua/src/*.c")
add_library(lua STATIC ${LUA_SRC})
set_target_properties(lua PROPERTIES EXCLUDE_FROM_ALL TRUE)

add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

target_link_options(shenanigans PRIVATE 
    "-L${CMAKE_SOURCE_DIR}/external/sdl3"
)

target_link_libraries(shenanigans PUBLIC
    OpenGL::GL
    glad
    SDL3
    lua
)

#copy DLL's onto output directory
add_custom_target(copy_runtime_dlls ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/external/sdl3/SDL3.dll"
        "${CMAKE_BINARY_DIR}"

)

add_custom_target(copy_assets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${CMAKE_BINARY_DIR}/assets"
)

add_dependencies(shenanigans 
    copy_runtime_dlls
    copy_assets
)
