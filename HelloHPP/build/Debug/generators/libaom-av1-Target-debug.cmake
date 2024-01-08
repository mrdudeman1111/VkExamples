# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(libaom-av1_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(libaom-av1_FRAMEWORKS_FOUND_DEBUG "${libaom-av1_FRAMEWORKS_DEBUG}" "${libaom-av1_FRAMEWORK_DIRS_DEBUG}")

set(libaom-av1_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET libaom-av1_DEPS_TARGET)
    add_library(libaom-av1_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET libaom-av1_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${libaom-av1_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${libaom-av1_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### libaom-av1_DEPS_TARGET to all of them
conan_package_library_targets("${libaom-av1_LIBS_DEBUG}"    # libraries
                              "${libaom-av1_LIB_DIRS_DEBUG}" # package_libdir
                              "${libaom-av1_BIN_DIRS_DEBUG}" # package_bindir
                              "${libaom-av1_LIBRARY_TYPE_DEBUG}"
                              "${libaom-av1_IS_HOST_WINDOWS_DEBUG}"
                              libaom-av1_DEPS_TARGET
                              libaom-av1_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "libaom-av1"    # package_name
                              "${libaom-av1_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${libaom-av1_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET libaom-av1::libaom-av1
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${libaom-av1_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${libaom-av1_LIBRARIES_TARGETS}>
                 APPEND)

    if("${libaom-av1_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET libaom-av1::libaom-av1
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     libaom-av1_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET libaom-av1::libaom-av1
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${libaom-av1_LINKER_FLAGS_DEBUG}> APPEND)
    set_property(TARGET libaom-av1::libaom-av1
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${libaom-av1_INCLUDE_DIRS_DEBUG}> APPEND)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET libaom-av1::libaom-av1
                 PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${libaom-av1_LIB_DIRS_DEBUG}> APPEND)
    set_property(TARGET libaom-av1::libaom-av1
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${libaom-av1_COMPILE_DEFINITIONS_DEBUG}> APPEND)
    set_property(TARGET libaom-av1::libaom-av1
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${libaom-av1_COMPILE_OPTIONS_DEBUG}> APPEND)

########## For the modules (FindXXX)
set(libaom-av1_LIBRARIES_DEBUG libaom-av1::libaom-av1)
