CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("Hello Triangle")

#set(CMAKE_BUILD_TYPE Debug)

file(GLOB SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

configure_file(Mesh.dae CubeMesh.dae COPYONLY)

add_executable(HelloCube ${SRC})

target_link_libraries(HelloCube vulkan glfw3 assimp)

