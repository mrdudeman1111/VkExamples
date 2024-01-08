# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(giflib_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(giflib_FRAMEWORKS_FOUND_DEBUG "${giflib_FRAMEWORKS_DEBUG}" "${giflib_FRAMEWORK_DIRS_DEBUG}")

set(giflib_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET giflib_DEPS_TARGET)
    add_library(giflib_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET giflib_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${giflib_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${giflib_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### giflib_DEPS_TARGET to all of them
conan_package_library_targets("${giflib_LIBS_DEBUG}"    # libraries
                              "${giflib_LIB_DIRS_DEBUG}" # package_libdir
                              "${giflib_BIN_DIRS_DEBUG}" # package_bindir
                              "${giflib_LIBRARY_TYPE_DEBUG}"
                              "${giflib_IS_HOST_WINDOWS_DEBUG}"
                              giflib_DEPS_TARGET
                              giflib_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "giflib"    # package_name
                              "${giflib_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${giflib_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET GIF::GIF
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${giflib_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${giflib_LIBRARIES_TARGETS}>
                 APPEND)

    if("${giflib_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET GIF::GIF
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     giflib_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET GIF::GIF
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${giflib_LINKER_FLAGS_DEBUG}> APPEND)
    set_property(TARGET GIF::GIF
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${giflib_INCLUDE_DIRS_DEBUG}> APPEND)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET GIF::GIF
                 PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${giflib_LIB_DIRS_DEBUG}> APPEND)
    set_property(TARGET GIF::GIF
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${giflib_COMPILE_DEFINITIONS_DEBUG}> APPEND)
    set_property(TARGET GIF::GIF
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${giflib_COMPILE_OPTIONS_DEBUG}> APPEND)

########## For the modules (FindXXX)
set(giflib_LIBRARIES_DEBUG GIF::GIF)
