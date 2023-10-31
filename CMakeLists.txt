CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("VkExamples")

set(CMAKE_BUILD_TYPE DEBUG)

include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/assimp/include)
include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/assimpBin/include)
include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/ThirdParty/glm)

if(UNIX)
  include_directories(/usr/local/include/sail/)
  link_directories(/usr/local/lib/)
endif()

link_directories(${CMAKE_SOURCE_DIR}/ThirdParty/assimpBin/bin)
link_directories(${CMAKE_SOURCE_DIR}/ThirdParty/glfwBin/src)

add_subdirectory(${CMAKE_SOURCE_DIR}/HelloTriangle)
add_subdirectory(${CMAKE_SOURCE_DIR}/HelloColorTriangle)
add_subdirectory(${CMAKE_SOURCE_DIR}/HelloCube)
add_subdirectory(${CMAKE_SOURCE_DIR}/HelloCubeCamera)
add_subdirectory(${CMAKE_SOURCE_DIR}/HelloTexturedCube)

