# Load the debug and release variables
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB DATA_FILES "${_DIR}/libaom-av1-*-data.cmake")

foreach(f ${DATA_FILES})
    include(${f})
endforeach()

# Create the targets for all the components
foreach(_COMPONENT ${libaom-av1_COMPONENT_NAMES} )
    if(NOT TARGET ${_COMPONENT})
        add_library(${_COMPONENT} INTERFACE IMPORTED)
        message(${libaom-av1_MESSAGE_MODE} "Conan: Component target declared '${_COMPONENT}'")
    endif()
endforeach()

if(NOT TARGET libaom-av1::libaom-av1)
    add_library(libaom-av1::libaom-av1 INTERFACE IMPORTED)
    message(${libaom-av1_MESSAGE_MODE} "Conan: Target declared 'libaom-av1::libaom-av1'")
endif()
# Load the debug and release library finders
get_filename_component(_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
file(GLOB CONFIG_FILES "${_DIR}/libaom-av1-Target-*.cmake")

foreach(f ${CONFIG_FILES})
    include(${f})
endforeach()