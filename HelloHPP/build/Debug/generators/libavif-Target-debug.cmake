# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(libavif_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(libavif_FRAMEWORKS_FOUND_DEBUG "${libavif_FRAMEWORKS_DEBUG}" "${libavif_FRAMEWORK_DIRS_DEBUG}")

set(libavif_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET libavif_DEPS_TARGET)
    add_library(libavif_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET libavif_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${libavif_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${libavif_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:libyuv::libyuv;libaom-av1::libaom-av1;dav1d::dav1d;WebP::sharpyuv>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### libavif_DEPS_TARGET to all of them
conan_package_library_targets("${libavif_LIBS_DEBUG}"    # libraries
                              "${libavif_LIB_DIRS_DEBUG}" # package_libdir
                              "${libavif_BIN_DIRS_DEBUG}" # package_bindir
                              "${libavif_LIBRARY_TYPE_DEBUG}"
                              "${libavif_IS_HOST_WINDOWS_DEBUG}"
                              libavif_DEPS_TARGET
                              libavif_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "libavif"    # package_name
                              "${libavif_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${libavif_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET avif
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${libavif_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${libavif_LIBRARIES_TARGETS}>
                 APPEND)

    if("${libavif_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET avif
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     libavif_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET avif
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${libavif_LINKER_FLAGS_DEBUG}> APPEND)
    set_property(TARGET avif
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${libavif_INCLUDE_DIRS_DEBUG}> APPEND)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET avif
                 PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${libavif_LIB_DIRS_DEBUG}> APPEND)
    set_property(TARGET avif
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${libavif_COMPILE_DEFINITIONS_DEBUG}> APPEND)
    set_property(TARGET avif
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${libavif_COMPILE_OPTIONS_DEBUG}> APPEND)

########## For the modules (FindXXX)
set(libavif_LIBRARIES_DEBUG avif)
