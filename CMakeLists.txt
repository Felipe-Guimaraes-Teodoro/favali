cmake_minimum_required(VERSION 3.10.0)
project(favali VERSION 0.1.0 LANGUAGES C CXX)

find_package(OpenGL REQUIRED)
find_package(Threads REQUIRED)

add_subdirectory("${CMAKE_SOURCE_DIR}/external/enet-master")

# ---------------- imgui ----------------
add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/backends/imgui_impl_sdl3.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
    external/imgui
    ${CMAKE_SOURCE_DIR}/external/sdl3/include
)

target_link_libraries(imgui PUBLIC SDL3)

# ---------------- lua ----------------
file(GLOB LUA_SRC "${CMAKE_SOURCE_DIR}/external/lua/src/*.c")
add_library(lua STATIC ${LUA_SRC})
set_target_properties(lua PROPERTIES EXCLUDE_FROM_ALL TRUE)

# ---------------- glad ----------------
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# ---------------- shared sources ----------------
set(FAVALI_SOURCES
    src/level.cpp

    src/physics/ik.cpp
    src/physics/sod.cpp
    src/physics/BVH.cpp
    src/physics/geometry.cpp
    src/physics/raycast.cpp

    src/graphics/camera.cpp
    src/graphics/mesh.cpp
    src/graphics/shaders.cpp
    src/graphics/texture.cpp
    src/graphics/model.cpp
    src/graphics/shapes.cpp
    src/graphics/gizmos.cpp
    src/graphics/sun.cpp

    src/gameplay/player.cpp
    src/gameplay/gun.cpp
    src/gameplay/bullet.cpp
    src/gameplay/scripting.c

    src/audio/audio.cpp
)

set(FAVALI_INCLUDES
    ${CMAKE_SOURCE_DIR}/external/include
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/sdl3/include
    ${CMAKE_SOURCE_DIR}/external/lua/src
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/enet-master/include

    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/graphics
    ${CMAKE_SOURCE_DIR}/src/physics
    ${CMAKE_SOURCE_DIR}/src/gameplay
    ${CMAKE_SOURCE_DIR}/src/audio
)

set(FAVALI_LIBS
    OpenGL::GL
    Threads::Threads
    glad
    SDL3
    lua
    imgui
    enet
)

# ---------------- executables ----------------
add_executable(favali
    src/main.cpp
    ${FAVALI_SOURCES}
)

add_executable(server
    src/server.cpp
    ${FAVALI_SOURCES}
)

add_executable(client
    src/client.cpp
    ${FAVALI_SOURCES}
)

foreach(target favali server client)
    target_include_directories(${target} PUBLIC ${FAVALI_INCLUDES})
    target_link_libraries(${target} PUBLIC ${FAVALI_LIBS})
    target_link_options(${target} PRIVATE "-L${CMAKE_SOURCE_DIR}/external/sdl3")
endforeach()

# ---------------- runtime stuff ----------------
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

add_dependencies(favali copy_runtime_dlls copy_assets)
add_dependencies(server copy_runtime_dlls copy_assets)
add_dependencies(client copy_runtime_dlls copy_assets)
