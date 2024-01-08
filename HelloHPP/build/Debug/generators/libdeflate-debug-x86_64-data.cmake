########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

list(APPEND libdeflate_COMPONENT_NAMES libdeflate::libdeflate_static)
list(REMOVE_DUPLICATES libdeflate_COMPONENT_NAMES)
set(libdeflate_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(libdeflate_PACKAGE_FOLDER_DEBUG "/home/ethan/.conan2/p/b/libde8410e06a517f3/p")
set(libdeflate_BUILD_MODULES_PATHS_DEBUG )


set(libdeflate_INCLUDE_DIRS_DEBUG )
set(libdeflate_RES_DIRS_DEBUG )
set(libdeflate_DEFINITIONS_DEBUG )
set(libdeflate_SHARED_LINK_FLAGS_DEBUG )
set(libdeflate_EXE_LINK_FLAGS_DEBUG )
set(libdeflate_OBJECTS_DEBUG )
set(libdeflate_COMPILE_DEFINITIONS_DEBUG )
set(libdeflate_COMPILE_OPTIONS_C_DEBUG )
set(libdeflate_COMPILE_OPTIONS_CXX_DEBUG )
set(libdeflate_LIB_DIRS_DEBUG "${libdeflate_PACKAGE_FOLDER_DEBUG}/lib")
set(libdeflate_BIN_DIRS_DEBUG )
set(libdeflate_LIBRARY_TYPE_DEBUG STATIC)
set(libdeflate_IS_HOST_WINDOWS_DEBUG 0)
set(libdeflate_LIBS_DEBUG deflate)
set(libdeflate_SYSTEM_LIBS_DEBUG )
set(libdeflate_FRAMEWORK_DIRS_DEBUG )
set(libdeflate_FRAMEWORKS_DEBUG )
set(libdeflate_BUILD_DIRS_DEBUG )
set(libdeflate_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(libdeflate_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${libdeflate_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${libdeflate_COMPILE_OPTIONS_C_DEBUG}>")
set(libdeflate_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libdeflate_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libdeflate_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libdeflate_EXE_LINK_FLAGS_DEBUG}>")


set(libdeflate_COMPONENTS_DEBUG libdeflate::libdeflate_static)
########### COMPONENT libdeflate::libdeflate_static VARIABLES ############################################

set(libdeflate_libdeflate_libdeflate_static_INCLUDE_DIRS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_LIB_DIRS_DEBUG "${libdeflate_PACKAGE_FOLDER_DEBUG}/lib")
set(libdeflate_libdeflate_libdeflate_static_BIN_DIRS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_LIBRARY_TYPE_DEBUG STATIC)
set(libdeflate_libdeflate_libdeflate_static_IS_HOST_WINDOWS_DEBUG 0)
set(libdeflate_libdeflate_libdeflate_static_RES_DIRS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_DEFINITIONS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_OBJECTS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_COMPILE_DEFINITIONS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_COMPILE_OPTIONS_C_DEBUG "")
set(libdeflate_libdeflate_libdeflate_static_COMPILE_OPTIONS_CXX_DEBUG "")
set(libdeflate_libdeflate_libdeflate_static_LIBS_DEBUG deflate)
set(libdeflate_libdeflate_libdeflate_static_SYSTEM_LIBS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_FRAMEWORK_DIRS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_FRAMEWORKS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_DEPENDENCIES_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_SHARED_LINK_FLAGS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_EXE_LINK_FLAGS_DEBUG )
set(libdeflate_libdeflate_libdeflate_static_NO_SONAME_MODE_DEBUG FALSE)

# COMPOUND VARIABLES
set(libdeflate_libdeflate_libdeflate_static_LINKER_FLAGS_DEBUG
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libdeflate_libdeflate_libdeflate_static_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libdeflate_libdeflate_libdeflate_static_SHARED_LINK_FLAGS_DEBUG}>
        $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libdeflate_libdeflate_libdeflate_static_EXE_LINK_FLAGS_DEBUG}>
)
set(libdeflate_libdeflate_libdeflate_static_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${libdeflate_libdeflate_libdeflate_static_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${libdeflate_libdeflate_libdeflate_static_COMPILE_OPTIONS_C_DEBUG}>")