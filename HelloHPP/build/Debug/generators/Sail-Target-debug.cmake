# Avoid multiple calls to find_package to append duplicated properties to the targets
include_guard()########### VARIABLES #######################################################################
#############################################################################################
set(sail_FRAMEWORKS_FOUND_DEBUG "") # Will be filled later
conan_find_apple_frameworks(sail_FRAMEWORKS_FOUND_DEBUG "${sail_FRAMEWORKS_DEBUG}" "${sail_FRAMEWORK_DIRS_DEBUG}")

set(sail_LIBRARIES_TARGETS "") # Will be filled later


######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
if(NOT TARGET sail_DEPS_TARGET)
    add_library(sail_DEPS_TARGET INTERFACE IMPORTED)
endif()

set_property(TARGET sail_DEPS_TARGET
             PROPERTY INTERFACE_LINK_LIBRARIES
             $<$<CONFIG:Debug>:${sail_FRAMEWORKS_FOUND_DEBUG}>
             $<$<CONFIG:Debug>:${sail_SYSTEM_LIBS_DEBUG}>
             $<$<CONFIG:Debug>:SAIL::SailCommon;GIF::GIF;JPEG::JPEG;PNG::PNG;TIFF::TIFF;avif;Jasper::Jasper;libwebp::libwebp;SAIL::SailCodecs;SAIL::Sail;SAIL::SailManip>
             APPEND)

####### Find the libraries declared in cpp_info.libs, create an IMPORTED target for each one and link the
####### sail_DEPS_TARGET to all of them
conan_package_library_targets("${sail_LIBS_DEBUG}"    # libraries
                              "${sail_LIB_DIRS_DEBUG}" # package_libdir
                              "${sail_BIN_DIRS_DEBUG}" # package_bindir
                              "${sail_LIBRARY_TYPE_DEBUG}"
                              "${sail_IS_HOST_WINDOWS_DEBUG}"
                              sail_DEPS_TARGET
                              sail_LIBRARIES_TARGETS  # out_libraries_targets
                              "_DEBUG"
                              "sail"    # package_name
                              "${sail_NO_SONAME_MODE_DEBUG}")  # soname

# FIXME: What is the result of this for multi-config? All configs adding themselves to path?
set(CMAKE_MODULE_PATH ${sail_BUILD_DIRS_DEBUG} ${CMAKE_MODULE_PATH})

########## COMPONENTS TARGET PROPERTIES Debug ########################################

    ########## COMPONENT SAIL::SailC++ #############

        set(sail_SAIL_SailC++_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(sail_SAIL_SailC++_FRAMEWORKS_FOUND_DEBUG "${sail_SAIL_SailC++_FRAMEWORKS_DEBUG}" "${sail_SAIL_SailC++_FRAMEWORK_DIRS_DEBUG}")

        set(sail_SAIL_SailC++_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET sail_SAIL_SailC++_DEPS_TARGET)
            add_library(sail_SAIL_SailC++_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET sail_SAIL_SailC++_DEPS_TARGET
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailC++_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailC++_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailC++_DEPENDENCIES_DEBUG}>
                     APPEND)

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'sail_SAIL_SailC++_DEPS_TARGET' to all of them
        conan_package_library_targets("${sail_SAIL_SailC++_LIBS_DEBUG}"
                              "${sail_SAIL_SailC++_LIB_DIRS_DEBUG}"
                              "${sail_SAIL_SailC++_BIN_DIRS_DEBUG}" # package_bindir
                              "${sail_SAIL_SailC++_LIBRARY_TYPE_DEBUG}"
                              "${sail_SAIL_SailC++_IS_HOST_WINDOWS_DEBUG}"
                              sail_SAIL_SailC++_DEPS_TARGET
                              sail_SAIL_SailC++_LIBRARIES_TARGETS
                              "_DEBUG"
                              "sail_SAIL_SailC++"
                              "${sail_SAIL_SailC++_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET SAIL::SailC++
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailC++_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailC++_LIBRARIES_TARGETS}>
                     APPEND)

        if("${sail_SAIL_SailC++_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET SAIL::SailC++
                         PROPERTY INTERFACE_LINK_LIBRARIES
                         sail_SAIL_SailC++_DEPS_TARGET
                         APPEND)
        endif()

        set_property(TARGET SAIL::SailC++ PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailC++_LINKER_FLAGS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailC++ PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailC++_INCLUDE_DIRS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailC++ PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailC++_LIB_DIRS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailC++ PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailC++_COMPILE_DEFINITIONS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailC++ PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailC++_COMPILE_OPTIONS_DEBUG}> APPEND)

    ########## COMPONENT SAIL::SailManip #############

        set(sail_SAIL_SailManip_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(sail_SAIL_SailManip_FRAMEWORKS_FOUND_DEBUG "${sail_SAIL_SailManip_FRAMEWORKS_DEBUG}" "${sail_SAIL_SailManip_FRAMEWORK_DIRS_DEBUG}")

        set(sail_SAIL_SailManip_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET sail_SAIL_SailManip_DEPS_TARGET)
            add_library(sail_SAIL_SailManip_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET sail_SAIL_SailManip_DEPS_TARGET
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailManip_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailManip_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailManip_DEPENDENCIES_DEBUG}>
                     APPEND)

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'sail_SAIL_SailManip_DEPS_TARGET' to all of them
        conan_package_library_targets("${sail_SAIL_SailManip_LIBS_DEBUG}"
                              "${sail_SAIL_SailManip_LIB_DIRS_DEBUG}"
                              "${sail_SAIL_SailManip_BIN_DIRS_DEBUG}" # package_bindir
                              "${sail_SAIL_SailManip_LIBRARY_TYPE_DEBUG}"
                              "${sail_SAIL_SailManip_IS_HOST_WINDOWS_DEBUG}"
                              sail_SAIL_SailManip_DEPS_TARGET
                              sail_SAIL_SailManip_LIBRARIES_TARGETS
                              "_DEBUG"
                              "sail_SAIL_SailManip"
                              "${sail_SAIL_SailManip_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET SAIL::SailManip
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailManip_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailManip_LIBRARIES_TARGETS}>
                     APPEND)

        if("${sail_SAIL_SailManip_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET SAIL::SailManip
                         PROPERTY INTERFACE_LINK_LIBRARIES
                         sail_SAIL_SailManip_DEPS_TARGET
                         APPEND)
        endif()

        set_property(TARGET SAIL::SailManip PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailManip_LINKER_FLAGS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailManip PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailManip_INCLUDE_DIRS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailManip PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailManip_LIB_DIRS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailManip PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailManip_COMPILE_DEFINITIONS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailManip PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailManip_COMPILE_OPTIONS_DEBUG}> APPEND)

    ########## COMPONENT SAIL::Sail #############

        set(sail_SAIL_Sail_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(sail_SAIL_Sail_FRAMEWORKS_FOUND_DEBUG "${sail_SAIL_Sail_FRAMEWORKS_DEBUG}" "${sail_SAIL_Sail_FRAMEWORK_DIRS_DEBUG}")

        set(sail_SAIL_Sail_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET sail_SAIL_Sail_DEPS_TARGET)
            add_library(sail_SAIL_Sail_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET sail_SAIL_Sail_DEPS_TARGET
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${sail_SAIL_Sail_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_Sail_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_Sail_DEPENDENCIES_DEBUG}>
                     APPEND)

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'sail_SAIL_Sail_DEPS_TARGET' to all of them
        conan_package_library_targets("${sail_SAIL_Sail_LIBS_DEBUG}"
                              "${sail_SAIL_Sail_LIB_DIRS_DEBUG}"
                              "${sail_SAIL_Sail_BIN_DIRS_DEBUG}" # package_bindir
                              "${sail_SAIL_Sail_LIBRARY_TYPE_DEBUG}"
                              "${sail_SAIL_Sail_IS_HOST_WINDOWS_DEBUG}"
                              sail_SAIL_Sail_DEPS_TARGET
                              sail_SAIL_Sail_LIBRARIES_TARGETS
                              "_DEBUG"
                              "sail_SAIL_Sail"
                              "${sail_SAIL_Sail_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET SAIL::Sail
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${sail_SAIL_Sail_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_Sail_LIBRARIES_TARGETS}>
                     APPEND)

        if("${sail_SAIL_Sail_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET SAIL::Sail
                         PROPERTY INTERFACE_LINK_LIBRARIES
                         sail_SAIL_Sail_DEPS_TARGET
                         APPEND)
        endif()

        set_property(TARGET SAIL::Sail PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_Sail_LINKER_FLAGS_DEBUG}> APPEND)
        set_property(TARGET SAIL::Sail PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${sail_SAIL_Sail_INCLUDE_DIRS_DEBUG}> APPEND)
        set_property(TARGET SAIL::Sail PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${sail_SAIL_Sail_LIB_DIRS_DEBUG}> APPEND)
        set_property(TARGET SAIL::Sail PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_Sail_COMPILE_DEFINITIONS_DEBUG}> APPEND)
        set_property(TARGET SAIL::Sail PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_Sail_COMPILE_OPTIONS_DEBUG}> APPEND)

    ########## COMPONENT SAIL::SailCodecs #############

        set(sail_SAIL_SailCodecs_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(sail_SAIL_SailCodecs_FRAMEWORKS_FOUND_DEBUG "${sail_SAIL_SailCodecs_FRAMEWORKS_DEBUG}" "${sail_SAIL_SailCodecs_FRAMEWORK_DIRS_DEBUG}")

        set(sail_SAIL_SailCodecs_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET sail_SAIL_SailCodecs_DEPS_TARGET)
            add_library(sail_SAIL_SailCodecs_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET sail_SAIL_SailCodecs_DEPS_TARGET
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCodecs_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCodecs_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCodecs_DEPENDENCIES_DEBUG}>
                     APPEND)

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'sail_SAIL_SailCodecs_DEPS_TARGET' to all of them
        conan_package_library_targets("${sail_SAIL_SailCodecs_LIBS_DEBUG}"
                              "${sail_SAIL_SailCodecs_LIB_DIRS_DEBUG}"
                              "${sail_SAIL_SailCodecs_BIN_DIRS_DEBUG}" # package_bindir
                              "${sail_SAIL_SailCodecs_LIBRARY_TYPE_DEBUG}"
                              "${sail_SAIL_SailCodecs_IS_HOST_WINDOWS_DEBUG}"
                              sail_SAIL_SailCodecs_DEPS_TARGET
                              sail_SAIL_SailCodecs_LIBRARIES_TARGETS
                              "_DEBUG"
                              "sail_SAIL_SailCodecs"
                              "${sail_SAIL_SailCodecs_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET SAIL::SailCodecs
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCodecs_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCodecs_LIBRARIES_TARGETS}>
                     APPEND)

        if("${sail_SAIL_SailCodecs_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET SAIL::SailCodecs
                         PROPERTY INTERFACE_LINK_LIBRARIES
                         sail_SAIL_SailCodecs_DEPS_TARGET
                         APPEND)
        endif()

        set_property(TARGET SAIL::SailCodecs PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCodecs_LINKER_FLAGS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailCodecs PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCodecs_INCLUDE_DIRS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailCodecs PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCodecs_LIB_DIRS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailCodecs PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCodecs_COMPILE_DEFINITIONS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailCodecs PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCodecs_COMPILE_OPTIONS_DEBUG}> APPEND)

    ########## COMPONENT SAIL::SailCommon #############

        set(sail_SAIL_SailCommon_FRAMEWORKS_FOUND_DEBUG "")
        conan_find_apple_frameworks(sail_SAIL_SailCommon_FRAMEWORKS_FOUND_DEBUG "${sail_SAIL_SailCommon_FRAMEWORKS_DEBUG}" "${sail_SAIL_SailCommon_FRAMEWORK_DIRS_DEBUG}")

        set(sail_SAIL_SailCommon_LIBRARIES_TARGETS "")

        ######## Create an interface target to contain all the dependencies (frameworks, system and conan deps)
        if(NOT TARGET sail_SAIL_SailCommon_DEPS_TARGET)
            add_library(sail_SAIL_SailCommon_DEPS_TARGET INTERFACE IMPORTED)
        endif()

        set_property(TARGET sail_SAIL_SailCommon_DEPS_TARGET
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCommon_FRAMEWORKS_FOUND_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCommon_SYSTEM_LIBS_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCommon_DEPENDENCIES_DEBUG}>
                     APPEND)

        ####### Find the libraries declared in cpp_info.component["xxx"].libs,
        ####### create an IMPORTED target for each one and link the 'sail_SAIL_SailCommon_DEPS_TARGET' to all of them
        conan_package_library_targets("${sail_SAIL_SailCommon_LIBS_DEBUG}"
                              "${sail_SAIL_SailCommon_LIB_DIRS_DEBUG}"
                              "${sail_SAIL_SailCommon_BIN_DIRS_DEBUG}" # package_bindir
                              "${sail_SAIL_SailCommon_LIBRARY_TYPE_DEBUG}"
                              "${sail_SAIL_SailCommon_IS_HOST_WINDOWS_DEBUG}"
                              sail_SAIL_SailCommon_DEPS_TARGET
                              sail_SAIL_SailCommon_LIBRARIES_TARGETS
                              "_DEBUG"
                              "sail_SAIL_SailCommon"
                              "${sail_SAIL_SailCommon_NO_SONAME_MODE_DEBUG}")


        ########## TARGET PROPERTIES #####################################
        set_property(TARGET SAIL::SailCommon
                     PROPERTY INTERFACE_LINK_LIBRARIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCommon_OBJECTS_DEBUG}>
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCommon_LIBRARIES_TARGETS}>
                     APPEND)

        if("${sail_SAIL_SailCommon_LIBS_DEBUG}" STREQUAL "")
            # If the component is not declaring any "cpp_info.components['foo'].libs" the system, frameworks etc are not
            # linked to the imported targets and we need to do it to the global target
            set_property(TARGET SAIL::SailCommon
                         PROPERTY INTERFACE_LINK_LIBRARIES
                         sail_SAIL_SailCommon_DEPS_TARGET
                         APPEND)
        endif()

        set_property(TARGET SAIL::SailCommon PROPERTY INTERFACE_LINK_OPTIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCommon_LINKER_FLAGS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailCommon PROPERTY INTERFACE_INCLUDE_DIRECTORIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCommon_INCLUDE_DIRS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailCommon PROPERTY INTERFACE_LINK_DIRECTORIES
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCommon_LIB_DIRS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailCommon PROPERTY INTERFACE_COMPILE_DEFINITIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCommon_COMPILE_DEFINITIONS_DEBUG}> APPEND)
        set_property(TARGET SAIL::SailCommon PROPERTY INTERFACE_COMPILE_OPTIONS
                     $<$<CONFIG:Debug>:${sail_SAIL_SailCommon_COMPILE_OPTIONS_DEBUG}> APPEND)

    ########## AGGREGATED GLOBAL TARGET WITH THE COMPONENTS #####################
    set_property(TARGET sail::sail PROPERTY INTERFACE_LINK_LIBRARIES SAIL::SailC++ APPEND)
    set_property(TARGET sail::sail PROPERTY INTERFACE_LINK_LIBRARIES SAIL::SailManip APPEND)
    set_property(TARGET sail::sail PROPERTY INTERFACE_LINK_LIBRARIES SAIL::Sail APPEND)
    set_property(TARGET sail::sail PROPERTY INTERFACE_LINK_LIBRARIES SAIL::SailCodecs APPEND)
    set_property(TARGET sail::sail PROPERTY INTERFACE_LINK_LIBRARIES SAIL::SailCommon APPEND)

########## For the modules (FindXXX)
set(sail_LIBRARIES_DEBUG sail::sail)
