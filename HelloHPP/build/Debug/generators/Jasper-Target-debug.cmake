# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(jasper_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(jasper_FRAMEWORKS_FOUND_DEBUG "${jasper_FRAMEWORKS_DEBUG}" "${jasper_FRAMEWORK_DIRS_DEBUG}")

set(jasper_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET jasper_DEPS_TARGET)
    add_library(jasper_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET jasper_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${jasper_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${jasper_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:JPEG::JPEG>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### jasper_DEPS_TARGET to all of them
conan_package_library_targets("${jasper_LIBS_DEBUG}"    # libraries
                              "${jasper_LIB_DIRS_DEBUG}" # package_libdir
                              "${jasper_BIN_DIRS_DEBUG}" # package_bindir
                              "${jasper_LIBRARY_TYPE_DEBUG}"
                              "${jasper_IS_HOST_WINDOWS_DEBUG}"
                              jasper_DEPS_TARGET
                              jasper_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "jasper"    # package_name
                              "${jasper_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${jasper_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET Jasper::Jasper
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${jasper_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${jasper_LIBRARIES_TARGETS}>
                 APPEND)

    if("${jasper_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET Jasper::Jasper
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     jasper_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET Jasper::Jasper
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${jasper_LINKER_FLAGS_DEBUG}> APPEND)
    set_property(TARGET Jasper::Jasper
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${jasper_INCLUDE_DIRS_DEBUG}> APPEND)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET Jasper::Jasper
                 PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${jasper_LIB_DIRS_DEBUG}> APPEND)
    set_property(TARGET Jasper::Jasper
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${jasper_COMPILE_DEFINITIONS_DEBUG}> APPEND)
    set_property(TARGET Jasper::Jasper
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${jasper_COMPILE_OPTIONS_DEBUG}> APPEND)

########## For the modules (FindXXX)
set(jasper_LIBRARIES_DEBUG Jasper::Jasper)
