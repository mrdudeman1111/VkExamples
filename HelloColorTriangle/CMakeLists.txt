CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("Hello Color Triangle")

#set(CMAKE_BUILD_TYPE Debug)

file(GLOB SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(HelloColorTriangle ${SRC})

target_link_libraries(HelloColorTriangle Vulkan::Vulkan glm::glm glfw)
