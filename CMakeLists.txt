CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("VkExamples")

set(CMAKE_BUILD_TYPE DEBUG)

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Sail REQUIRED)
find_package(assimp REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(${Vulkan_INCLUDE_DIRS} ${assimp_INCLUDE_DIRS} ${Sail_INCLUDE_DIRS} ${glm_INCLUDE_DIRS} ${glfw3_INCLUDE_DIRS})

if(Vulkan_FOUND)
  add_subdirectory(${CMAKE_SOURCE_DIR}/HelloTriangle)
  add_subdirectory(${CMAKE_SOURCE_DIR}/HelloColorTriangle)
  add_subdirectory(${CMAKE_SOURCE_DIR}/HelloCube)
  add_subdirectory(${CMAKE_SOURCE_DIR}/HelloCubeCamera)
  add_subdirectory(${CMAKE_SOURCE_DIR}/HelloTextures)
  add_subdirectory(${CMAKE_SOURCE_DIR}/HelloLit)
else()
  message(cmake failed to find vulkan, build failed)
endif()

