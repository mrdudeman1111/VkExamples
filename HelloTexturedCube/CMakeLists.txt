CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)
project("Hello Triangle")

file(GLOB SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

configure_file(Mesh.dae Mesh.dae COPYONLY)
configure_file(Mesh.png Mesh.png COPYONLY)

add_executable(HelloTextureCamera ${SRC})

if(MSVC)
  file(GLOB SAIL
      ${CMAKE_SOURCE_DIR}/ThirdParty/lib/libsail.dll)
else(UNIX)
  file(GLOB SAIL
      ${CMAKE_SOURCE_DIR}/ThirdParty/lib/libsail.so)
endif()

target_link_libraries(HelloTextureCamera vulkan glfw3 assimp sail sail-common sail-manip)

