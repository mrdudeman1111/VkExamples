########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND zstd_COMPONENT_NAMES zstd::libzstd_static)
list(REMOVE_DUPLICATES zstd_COMPONENT_NAMES)
set(zstd_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(zstd_PACKAGE_FOLDER_DEBUG "/home/ethan/.conan2/p/b/zstd3e65d57b8617a/p")
set(zstd_BUILD_MODULES_PATHS_DEBUG )


set(zstd_INCLUDE_DIRS_DEBUG )
set(zstd_RES_DIRS_DEBUG )
set(zstd_DEFINITIONS_DEBUG )
set(zstd_SHARED_LINK_FLAGS_DEBUG )
set(zstd_EXE_LINK_FLAGS_DEBUG )
set(zstd_OBJECTS_DEBUG )
set(zstd_COMPILE_DEFINITIONS_DEBUG )
set(zstd_COMPILE_OPTIONS_C_DEBUG )
set(zstd_COMPILE_OPTIONS_CXX_DEBUG )
set(zstd_LIB_DIRS_DEBUG "${zstd_PACKAGE_FOLDER_DEBUG}/lib")
set(zstd_BIN_DIRS_DEBUG )
set(zstd_LIBRARY_TYPE_DEBUG STATIC)
set(zstd_IS_HOST_WINDOWS_DEBUG 0)
set(zstd_LIBS_DEBUG zstd)
set(zstd_SYSTEM_LIBS_DEBUG pthread)
set(zstd_FRAMEWORK_DIRS_DEBUG )
set(zstd_FRAMEWORKS_DEBUG )
set(zstd_BUILD_DIRS_DEBUG )
set(zstd_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(zstd_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${zstd_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${zstd_COMPILE_OPTIONS_C_DEBUG}>")
set(zstd_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${zstd_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${zstd_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${zstd_EXE_LINK_FLAGS_DEBUG}>")


set(zstd_COMPONENTS_DEBUG zstd::libzstd_static)
########### COMPONENT zstd::libzstd_static VARIABLES ############################################

set(zstd_zstd_libzstd_static_INCLUDE_DIRS_DEBUG )
set(zstd_zstd_libzstd_static_LIB_DIRS_DEBUG "${zstd_PACKAGE_FOLDER_DEBUG}/lib")
set(zstd_zstd_libzstd_static_BIN_DIRS_DEBUG )
set(zstd_zstd_libzstd_static_LIBRARY_TYPE_DEBUG STATIC)
set(zstd_zstd_libzstd_static_IS_HOST_WINDOWS_DEBUG 0)
set(zstd_zstd_libzstd_static_RES_DIRS_DEBUG )
set(zstd_zstd_libzstd_static_DEFINITIONS_DEBUG )
set(zstd_zstd_libzstd_static_OBJECTS_DEBUG )
set(zstd_zstd_libzstd_static_COMPILE_DEFINITIONS_DEBUG )
set(zstd_zstd_libzstd_static_COMPILE_OPTIONS_C_DEBUG "")
set(zstd_zstd_libzstd_static_COMPILE_OPTIONS_CXX_DEBUG "")
set(zstd_zstd_libzstd_static_LIBS_DEBUG zstd)
set(zstd_zstd_libzstd_static_SYSTEM_LIBS_DEBUG pthread)
set(zstd_zstd_libzstd_static_FRAMEWORK_DIRS_DEBUG )
set(zstd_zstd_libzstd_static_FRAMEWORKS_DEBUG )
set(zstd_zstd_libzstd_static_DEPENDENCIES_DEBUG )
set(zstd_zstd_libzstd_static_SHARED_LINK_FLAGS_DEBUG )
set(zstd_zstd_libzstd_static_EXE_LINK_FLAGS_DEBUG )
set(zstd_zstd_libzstd_static_NO_SONAME_MODE_DEBUG FALSE)

# COMPOUND VARIABLES
set(zstd_zstd_libzstd_static_LINKER_FLAGS_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${zstd_zstd_libzstd_static_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${zstd_zstd_libzstd_static_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${zstd_zstd_libzstd_static_EXE_LINK_FLAGS_DEBUG}>
)
set(zstd_zstd_libzstd_static_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${zstd_zstd_libzstd_static_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${zstd_zstd_libzstd_static_COMPILE_OPTIONS_C_DEBUG}>")