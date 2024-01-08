CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("Hello Textures")

file(GLOB SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

configure_file(Mesh.dae Mesh.dae COPYONLY)
configure_file(Mesh.png Mesh.png COPYONLY)

add_executable(HelloTextures ${SRC})

target_link_libraries(HelloTextures Vulkan::Vulkan glfw assimp::assimp sail::sail)

