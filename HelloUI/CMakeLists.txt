CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("UI Renderer")

find_package(freetype REQUIRED)
find_package(pugixml REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Shaders)

add_executable(uiRender ${SOURCES})

target_link_libraries(uiRender vulkan assimp::assimp pugixml::pugixml Freetype::Freetype ${glm_LIBRARIES} ${glfw3_LIBRARIES})

target_compile_definitions(uiRender PUBLIC FONT="${CMAKE_CURRENT_SOURCE_DIR}/Raleway-Regular.ttf")
target_compile_definitions(uiRender PUBLIC SHADERPATH="${CMAKE_CURRENT_BINARY_DIR}/Shaders/")

