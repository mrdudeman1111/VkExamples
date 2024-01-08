# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(libdeflate_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(libdeflate_FRAMEWORKS_FOUND_DEBUG "${libdeflate_FRAMEWORKS_DEBUG}" "${libdeflate_FRAMEWORK_DIRS_DEBUG}")

set(libdeflate_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET libdeflate_DEPS_TARGET)
    add_library(libdeflate_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET libdeflate_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${libdeflate_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${libdeflate_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### libdeflate_DEPS_TARGET to all of them
conan_package_library_targets("${libdeflate_LIBS_DEBUG}"    # libraries
                              "${libdeflate_LIB_DIRS_DEBUG}" # package_libdir
                              "${libdeflate_BIN_DIRS_DEBUG}" # package_bindir
                              "${libdeflate_LIBRARY_TYPE_DEBUG}"
                              "${libdeflate_IS_HOST_WINDOWS_DEBUG}"
                              libdeflate_DEPS_TARGET
                              libdeflate_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "libdeflate"    # package_name
                              "${libdeflate_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${libdeflate_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Debug ########################################

    ########## COMPONENT libdeflate::libdeflate_static #############

        set(libdeflate_libdeflate_libdeflate_static_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(libdeflate_libdeflate_libdeflate_static_FRAMEWORKS_FOUND_DEBUG "${libdeflate_libdeflate_libdeflate_static_FRAMEWORKS_DEBUG}" "${libdeflate_libdeflate_libdeflate_static_FRAMEWORK_DIRS_DEBUG}")

        set(libdeflate_libdeflate_libdeflate_static_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET libdeflate_libdeflate_libdeflate_static_DEPS_TARGET)
            add_library(libdeflate_libdeflate_libdeflate_static_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET libdeflate_libdeflate_libdeflate_static_DEPS_TARGET
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${libdeflate_libdeflate_libdeflate_static_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${libdeflate_libdeflate_libdeflate_static_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${libdeflate_libdeflate_libdeflate_static_DEPENDENCIES_DEBUG}>
                     APPEND)

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'libdeflate_libdeflate_libdeflate_static_DEPS_TARGET' to all of them
        conan_package_library_targets("${libdeflate_libdeflate_libdeflate_static_LIBS_DEBUG}"
                              "${libdeflate_libdeflate_libdeflate_static_LIB_DIRS_DEBUG}"
                              "${libdeflate_libdeflate_libdeflate_static_BIN_DIRS_DEBUG}" # package_bindir
                              "${libdeflate_libdeflate_libdeflate_static_LIBRARY_TYPE_DEBUG}"
                              "${libdeflate_libdeflate_libdeflate_static_IS_HOST_WINDOWS_DEBUG}"
                              libdeflate_libdeflate_libdeflate_static_DEPS_TARGET
                              libdeflate_libdeflate_libdeflate_static_LIBRARIES_TARGETS
                              "_DEBUG"
                              "libdeflate_libdeflate_libdeflate_static"
                              "${libdeflate_libdeflate_libdeflate_static_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET libdeflate::libdeflate_static
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${libdeflate_libdeflate_libdeflate_static_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${libdeflate_libdeflate_libdeflate_static_LIBRARIES_TARGETS}>
                     APPEND)

        if("${libdeflate_libdeflate_libdeflate_static_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET libdeflate::libdeflate_static
                         PROPERTY INTERFACE_LINK_LIBRARIES
                         libdeflate_libdeflate_libdeflate_static_DEPS_TARGET
                         APPEND)
        endif()

        set_property(TARGET libdeflate::libdeflate_static PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${libdeflate_libdeflate_libdeflate_static_LINKER_FLAGS_DEBUG}> APPEND)
        set_property(TARGET libdeflate::libdeflate_static PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${libdeflate_libdeflate_libdeflate_static_INCLUDE_DIRS_DEBUG}> APPEND)
        set_property(TARGET libdeflate::libdeflate_static PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${libdeflate_libdeflate_libdeflate_static_LIB_DIRS_DEBUG}> APPEND)
        set_property(TARGET libdeflate::libdeflate_static PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${libdeflate_libdeflate_libdeflate_static_COMPILE_DEFINITIONS_DEBUG}> APPEND)
        set_property(TARGET libdeflate::libdeflate_static PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${libdeflate_libdeflate_libdeflate_static_COMPILE_OPTIONS_DEBUG}> APPEND)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET libdeflate::libdeflate_static PROPERTY INTERFACE_LINK_LIBRARIES libdeflate::libdeflate_static APPEND)

########## For the modules (FindXXX)
set(libdeflate_LIBRARIES_DEBUG libdeflate::libdeflate_static)
