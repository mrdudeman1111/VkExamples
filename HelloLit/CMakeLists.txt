CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("Hello lit")

file(GLOB SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

configure_file(Mesh.dae Mesh.dae COPYONLY)
configure_file(Mesh.png Mesh.png COPYONLY)

add_executable(HelloLit ${SRC})

target_include_directories(HelloLit PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(HelloLit Vulkan::Vulkan glfw assimp::assimp sail::sail)

