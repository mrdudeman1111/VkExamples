CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("Hello Triangle")

#set(CMAKE_BUILD_TYPE Debug)

file(GLOB SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(HelloColorTriangle ${SRC})

target_link_libraries(HelloColorTriangle vulkan glfw3)
