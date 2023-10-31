CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("Hello Triangle")

#set(CMAKE_BUILD_TYPE Debug)

file(GLOB SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

configure_file(Mesh.dae CubeCameraMesh.dae)

add_executable(HelloCubeCamera ${SRC})

target_link_libraries(HelloCubeCamera vulkan glfw3 assimp)

