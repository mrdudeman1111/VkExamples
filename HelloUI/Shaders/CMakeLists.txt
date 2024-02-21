CMAKE_MINIMUM_REQUIRED(VERSION 3.18.1)

file(GLOB uiShaderSources
    ${CMAKE_CURRENT_SOURCE_DIR}/*.glsl)

foreach(shader IN LISTS uiShaderSources)
  get_filename_component(shaderName ${shader} NAME_WLE)
  set(output "${CMAKE_CURRENT_BINARY_DIR}/${shaderName}.spv")
  message("Vulkan::glslc ${shader} -o ${output}")
  add_custom_command(
    COMMAND Vulkan::glslc ${shader} -o ${output}
    OUTPUT ${output}
    COMMENT "Compiling ${shader}"
    DEPENDS ${shader}
    )
  list(APPEND shaderBin ${output})
endforeach()

add_custom_target(uiShaders ALL DEPENDS ${shaderBin})

