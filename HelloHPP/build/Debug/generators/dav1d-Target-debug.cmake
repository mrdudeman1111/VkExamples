# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(dav1d_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(dav1d_FRAMEWORKS_FOUND_DEBUG "${dav1d_FRAMEWORKS_DEBUG}" "${dav1d_FRAMEWORK_DIRS_DEBUG}")

set(dav1d_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET dav1d_DEPS_TARGET)
    add_library(dav1d_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET dav1d_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${dav1d_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${dav1d_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### dav1d_DEPS_TARGET to all of them
conan_package_library_targets("${dav1d_LIBS_DEBUG}"    # libraries
                              "${dav1d_LIB_DIRS_DEBUG}" # package_libdir
                              "${dav1d_BIN_DIRS_DEBUG}" # package_bindir
                              "${dav1d_LIBRARY_TYPE_DEBUG}"
                              "${dav1d_IS_HOST_WINDOWS_DEBUG}"
                              dav1d_DEPS_TARGET
                              dav1d_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "dav1d"    # package_name
                              "${dav1d_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${dav1d_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## GLOBAL TARGET PROPERTIES Debug ########################################
    set_property(TARGET dav1d::dav1d
                 PROPERTY INTERFACE_LINK_LIBRARIES
                 $<$<CONFIG:Debug>:${dav1d_OBJECTS_DEBUG}>
                 $<$<CONFIG:Debug>:${dav1d_LIBRARIES_TARGETS}>
                 APPEND)

    if("${dav1d_LIBS_DEBUG}" STREQUAL "")
        # If the package is not declaring any "cpp_info.libs" the package deps, system libs,
        # frameworks etc are not linked to the imported targets and we need to do it to the
        # global target
        set_property(TARGET dav1d::dav1d
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     dav1d_DEPS_TARGET
                     APPEND)
    endif()

    set_property(TARGET dav1d::dav1d
                 PROPERTY INTERFACE_LINK_OPTIONS
                 $<$<CONFIG:Debug>:${dav1d_LINKER_FLAGS_DEBUG}> APPEND)
    set_property(TARGET dav1d::dav1d
                 PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                 $<$<CONFIG:Debug>:${dav1d_INCLUDE_DIRS_DEBUG}> APPEND)
    # Necessary to find LINK shared libraries in Linux
    set_property(TARGET dav1d::dav1d
                 PROPERTY INTERFACE_LINK_DIRECTORIES
                 $<$<CONFIG:Debug>:${dav1d_LIB_DIRS_DEBUG}> APPEND)
    set_property(TARGET dav1d::dav1d
                 PROPERTY INTERFACE_COMPILE_DEFINITIONS
                 $<$<CONFIG:Debug>:${dav1d_COMPILE_DEFINITIONS_DEBUG}> APPEND)
    set_property(TARGET dav1d::dav1d
                 PROPERTY INTERFACE_COMPILE_OPTIONS
                 $<$<CONFIG:Debug>:${dav1d_COMPILE_OPTIONS_DEBUG}> APPEND)

########## For the modules (FindXXX)
set(dav1d_LIBRARIES_DEBUG dav1d::dav1d)
