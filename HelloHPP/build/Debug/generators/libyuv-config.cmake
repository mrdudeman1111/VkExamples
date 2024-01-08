########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(libyuv_FIND_QUIETLY)
    set(libyuv_MESSAGE_MODE VERBOSE)
else()
    set(libyuv_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/libyuvTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${libyuv_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(libyuv_VERSION_STRING "1854")
set(libyuv_INCLUDE_DIRS ${libyuv_INCLUDE_DIRS_DEBUG} )
set(libyuv_INCLUDE_DIR ${libyuv_INCLUDE_DIRS_DEBUG} )
set(libyuv_LIBRARIES ${libyuv_LIBRARIES_DEBUG} )
set(libyuv_DEFINITIONS ${libyuv_DEFINITIONS_DEBUG} )

# Only the first installed configuration is included to avoid the collision
foreach(_BUILD_MODULE ${libyuv_BUILD_MODULES_PATHS_DEBUG} )
    message(${libyuv_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


