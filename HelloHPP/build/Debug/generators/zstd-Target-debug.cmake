# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(zstd_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(zstd_FRAMEWORKS_FOUND_DEBUG "${zstd_FRAMEWORKS_DEBUG}" "${zstd_FRAMEWORK_DIRS_DEBUG}")

set(zstd_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET zstd_DEPS_TARGET)
    add_library(zstd_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET zstd_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${zstd_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${zstd_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### zstd_DEPS_TARGET to all of them
conan_package_library_targets("${zstd_LIBS_DEBUG}"    # libraries
                              "${zstd_LIB_DIRS_DEBUG}" # package_libdir
                              "${zstd_BIN_DIRS_DEBUG}" # package_bindir
                              "${zstd_LIBRARY_TYPE_DEBUG}"
                              "${zstd_IS_HOST_WINDOWS_DEBUG}"
                              zstd_DEPS_TARGET
                              zstd_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "zstd"    # package_name
                              "${zstd_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${zstd_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Debug ########################################

    ########## COMPONENT zstd::libzstd_static #############

        set(zstd_zstd_libzstd_static_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(zstd_zstd_libzstd_static_FRAMEWORKS_FOUND_DEBUG "${zstd_zstd_libzstd_static_FRAMEWORKS_DEBUG}" "${zstd_zstd_libzstd_static_FRAMEWORK_DIRS_DEBUG}")

        set(zstd_zstd_libzstd_static_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET zstd_zstd_libzstd_static_DEPS_TARGET)
            add_library(zstd_zstd_libzstd_static_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET zstd_zstd_libzstd_static_DEPS_TARGET
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${zstd_zstd_libzstd_static_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${zstd_zstd_libzstd_static_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${zstd_zstd_libzstd_static_DEPENDENCIES_DEBUG}>
                     APPEND)

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'zstd_zstd_libzstd_static_DEPS_TARGET' to all of them
        conan_package_library_targets("${zstd_zstd_libzstd_static_LIBS_DEBUG}"
                              "${zstd_zstd_libzstd_static_LIB_DIRS_DEBUG}"
                              "${zstd_zstd_libzstd_static_BIN_DIRS_DEBUG}" # package_bindir
                              "${zstd_zstd_libzstd_static_LIBRARY_TYPE_DEBUG}"
                              "${zstd_zstd_libzstd_static_IS_HOST_WINDOWS_DEBUG}"
                              zstd_zstd_libzstd_static_DEPS_TARGET
                              zstd_zstd_libzstd_static_LIBRARIES_TARGETS
                              "_DEBUG"
                              "zstd_zstd_libzstd_static"
                              "${zstd_zstd_libzstd_static_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET zstd::libzstd_static
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${zstd_zstd_libzstd_static_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${zstd_zstd_libzstd_static_LIBRARIES_TARGETS}>
                     APPEND)

        if("${zstd_zstd_libzstd_static_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET zstd::libzstd_static
                         PROPERTY INTERFACE_LINK_LIBRARIES
                         zstd_zstd_libzstd_static_DEPS_TARGET
                         APPEND)
        endif()

        set_property(TARGET zstd::libzstd_static PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${zstd_zstd_libzstd_static_LINKER_FLAGS_DEBUG}> APPEND)
        set_property(TARGET zstd::libzstd_static PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${zstd_zstd_libzstd_static_INCLUDE_DIRS_DEBUG}> APPEND)
        set_property(TARGET zstd::libzstd_static PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${zstd_zstd_libzstd_static_LIB_DIRS_DEBUG}> APPEND)
        set_property(TARGET zstd::libzstd_static PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${zstd_zstd_libzstd_static_COMPILE_DEFINITIONS_DEBUG}> APPEND)
        set_property(TARGET zstd::libzstd_static PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${zstd_zstd_libzstd_static_COMPILE_OPTIONS_DEBUG}> APPEND)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET zstd::libzstd_static PROPERTY INTERFACE_LINK_LIBRARIES zstd::libzstd_static APPEND)

########## For the modules (FindXXX)
set(zstd_LIBRARIES_DEBUG zstd::libzstd_static)
