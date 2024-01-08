########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(libavif_FIND_QUIETLY)
    set(libavif_MESSAGE_MODE VERBOSE)
else()
    set(libavif_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/libavifTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${libavif_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(libavif_VERSION_STRING "1.0.2")
set(libavif_INCLUDE_DIRS ${libavif_INCLUDE_DIRS_DEBUG} )
set(libavif_INCLUDE_DIR ${libavif_INCLUDE_DIRS_DEBUG} )
set(libavif_LIBRARIES ${libavif_LIBRARIES_DEBUG} )
set(libavif_DEFINITIONS ${libavif_DEFINITIONS_DEBUG} )

# Only the first installed configuration is included to avoid the collision
foreach(_BUILD_MODULE ${libavif_BUILD_MODULES_PATHS_DEBUG} )
    message(${libavif_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


