cmake_minimum_required(VERSION 3.10.0)
project(shenanigans VERSION 0.1.0 LANGUAGES C CXX)

add_library(enet STATIC
    external/enet-master/host.c
    external/enet-master/peer.c
    external/enet-master/protocol.c
    external/enet-master/packet.c
    external/enet-master/list.c
    external/enet-master/compress.c
    external/enet-master/callbacks.c
)

if (WIN32)
    target_sources(enet PRIVATE external/enet-master/win32.c)
    target_link_libraries(enet PUBLIC ws2_32)
else()
    target_sources(enet PRIVATE external/enet-master/unix.c)
endif()

target_include_directories(enet PUBLIC
    external/enet-master/include
)

find_package(OpenGL REQUIRED)

add_library(imgui
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/imgui_tables.cpp

    external/imgui/backends/imgui_impl_sdl3.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC external/imgui)

add_executable(shenanigans 
    src/main.cpp
    src/scripting.c
    src/level.cpp
    src/bullet.cpp
    src/BVH.cpp
    src/geometry.cpp
    src/raycast.cpp
    src/player.cpp
    src/sod.cpp
    src/gun.cpp
    src/ik.cpp
    src/audio.cpp
    src/npc.cpp

    src/graphics/camera.cpp
    src/graphics/mesh.cpp
    src/graphics/shaders.cpp
    src/graphics/texture.cpp
    src/graphics/model.cpp
    src/graphics/shapes.cpp
    src/graphics/gizmos.cpp
    src/graphics/sun.cpp
)

target_include_directories(imgui PUBLIC
    ${CMAKE_SOURCE_DIR}/external/sdl3/include
)

target_include_directories(shenanigans PUBLIC
    ${CMAKE_SOURCE_DIR}/external/include
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/sdl3/include
    ${CMAKE_SOURCE_DIR}/external/lua/src
    ${CMAKE_SOURCE_DIR}/external/imgui

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

target_link_libraries(imgui PUBLIC
    SDL3
)

target_link_libraries(shenanigans PUBLIC
    OpenGL::GL
    glad
    SDL3
    lua
    imgui
    enet
)

if (WIN32)
target_link_libraries(shenanigans PUBLIC ws2_32)
endif()


# server and client stuff ----------------------------------
add_executable(shenanigans_server
	src/net/server.cpp
    src/scripting.c
    src/level.cpp
    src/bullet.cpp
    src/BVH.cpp
    src/geometry.cpp
    src/raycast.cpp
    src/player.cpp
    src/sod.cpp
    src/gun.cpp
    src/ik.cpp
    src/audio.cpp
    src/npc.cpp

    src/graphics/camera.cpp
    src/graphics/mesh.cpp
    src/graphics/shaders.cpp
    src/graphics/texture.cpp
    src/graphics/model.cpp
    src/graphics/shapes.cpp
    src/graphics/gizmos.cpp
    src/graphics/sun.cpp
)
target_link_libraries(shenanigans_server PUBLIC
    OpenGL::GL
    glad
    SDL3
    lua
    imgui
    enet
)
target_include_directories(shenanigans_server PUBLIC
    ${CMAKE_SOURCE_DIR}/external/enet-master/include
    ${CMAKE_SOURCE_DIR}/external/include
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/sdl3/include
    ${CMAKE_SOURCE_DIR}/external/lua/src
    ${CMAKE_SOURCE_DIR}/external/imgui

    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/graphics
    ${CMAKE_SOURCE_DIR}/src/audio
    ${CMAKE_SOURCE_DIR}/src/npc
    ${CMAKE_SOURCE_DIR}/src/level
)

target_link_directories(shenanigans_server PRIVATE
    ${CMAKE_SOURCE_DIR}/external/sdl3
)

add_executable(shenanigans_client
	src/net/client.cpp
    src/scripting.c
    src/level.cpp
    src/bullet.cpp
    src/BVH.cpp
    src/geometry.cpp
    src/raycast.cpp
    src/player.cpp
    src/sod.cpp
    src/gun.cpp
    src/ik.cpp
    src/audio.cpp
    src/npc.cpp

    src/graphics/camera.cpp
    src/graphics/mesh.cpp
    src/graphics/shaders.cpp
    src/graphics/texture.cpp
    src/graphics/model.cpp
    src/graphics/shapes.cpp
    src/graphics/gizmos.cpp
    src/graphics/sun.cpp
)
target_link_libraries(shenanigans_client PUBLIC
    OpenGL::GL
    glad
    SDL3
    lua
    imgui
    enet
)
target_include_directories(shenanigans_client PUBLIC
    ${CMAKE_SOURCE_DIR}/external/enet-master/include
    ${CMAKE_SOURCE_DIR}/external/include
    ${CMAKE_SOURCE_DIR}/external/glad/include
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/sdl3/include
    ${CMAKE_SOURCE_DIR}/external/lua/src
    ${CMAKE_SOURCE_DIR}/external/imgui

    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/graphics
    ${CMAKE_SOURCE_DIR}/src/audio
    ${CMAKE_SOURCE_DIR}/src/npc
    ${CMAKE_SOURCE_DIR}/src/level
)

target_link_directories(shenanigans_client PRIVATE
    ${CMAKE_SOURCE_DIR}/external/sdl3
)

target_compile_definitions(shenanigans_server PUBLIC HEADLESS)

if (WIN32)
	target_link_libraries(shenanigans_server PUBLIC ws2_32 winmm)
	target_link_libraries(shenanigans_client PUBLIC ws2_32 winmm)
endif()
# end of net -----------------------------------------------


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
