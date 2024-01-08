CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("vulkan_HPP")

find_package(Sail REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)

include_directories(${glfw3_INCLUDE_DIRS} ${glm_INCLUDE_DIRS} ${assimp_INCLUDE_DIRS})

file(GLOB SOURCES
    ${CMAKE_SOURCE_DIR}/*.cpp)

add_executable(hppExamples ${SOURCES})

target_link_libraries(hppExamples vulkan sail::sail ${glfw3_LIBRARIES} ${glm_LIBRARIES} ${assimp_LIBRARIES})

