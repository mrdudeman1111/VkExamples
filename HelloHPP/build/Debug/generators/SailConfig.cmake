########## MACROS ###########################################################################
#############################################################################################

# Requires CMake > 3.15
if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeDeps' generator only works with CMake >= 3.15")
endif()

if(Sail_FIND_QUIETLY)
    set(Sail_MESSAGE_MODE VERBOSE)
else()
    set(Sail_MESSAGE_MODE STATUS)
endif()

include(${CMAKE_CURRENT_LIST_DIR}/cmakedeps_macros.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/SailTargets.cmake)
include(CMakeFindDependencyMacro)

check_build_type_defined()

foreach(_DEPENDENCY ${sail_FIND_DEPENDENCY_NAMES} )
    # Check that we have not already called a find_package with the transitive dependency
    if(NOT ${_DEPENDENCY}_FOUND)
        find_dependency(${_DEPENDENCY} REQUIRED ${${_DEPENDENCY}_FIND_MODE})
    endif()
endforeach()

set(Sail_VERSION_STRING "0.9.0")
set(Sail_INCLUDE_DIRS ${sail_INCLUDE_DIRS_DEBUG} )
set(Sail_INCLUDE_DIR ${sail_INCLUDE_DIRS_DEBUG} )
set(Sail_LIBRARIES ${sail_LIBRARIES_DEBUG} )
set(Sail_DEFINITIONS ${sail_DEFINITIONS_DEBUG} )

# Only the first installed configuration is included to avoid the collision
foreach(_BUILD_MODULE ${sail_BUILD_MODULES_PATHS_DEBUG} )
    message(${Sail_MESSAGE_MODE} "Conan: Including build module from '${_BUILD_MODULE}'")
    include(${_BUILD_MODULE})
endforeach()


