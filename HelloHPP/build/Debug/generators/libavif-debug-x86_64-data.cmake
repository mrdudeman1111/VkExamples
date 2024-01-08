########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(libavif_COMPONENT_NAMES "")
list(APPEND libavif_FIND_DEPENDENCY_NAMES libaom-av1 libyuv dav1d WebP)
list(REMOVE_DUPLICATES libavif_FIND_DEPENDENCY_NAMES)
set(libaom-av1_FIND_MODE "NO_MODULE")
set(libyuv_FIND_MODE "NO_MODULE")
set(dav1d_FIND_MODE "NO_MODULE")
set(WebP_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(libavif_PACKAGE_FOLDER_DEBUG "/home/ethan/.conan2/p/b/libav993a76bd6686f/p")
set(libavif_BUILD_MODULES_PATHS_DEBUG )


set(libavif_INCLUDE_DIRS_DEBUG )
set(libavif_RES_DIRS_DEBUG )
set(libavif_DEFINITIONS_DEBUG )
set(libavif_SHARED_LINK_FLAGS_DEBUG )
set(libavif_EXE_LINK_FLAGS_DEBUG )
set(libavif_OBJECTS_DEBUG )
set(libavif_COMPILE_DEFINITIONS_DEBUG )
set(libavif_COMPILE_OPTIONS_C_DEBUG )
set(libavif_COMPILE_OPTIONS_CXX_DEBUG )
set(libavif_LIB_DIRS_DEBUG "${libavif_PACKAGE_FOLDER_DEBUG}/lib")
set(libavif_BIN_DIRS_DEBUG )
set(libavif_LIBRARY_TYPE_DEBUG STATIC)
set(libavif_IS_HOST_WINDOWS_DEBUG 0)
set(libavif_LIBS_DEBUG avif)
set(libavif_SYSTEM_LIBS_DEBUG pthread m dl)
set(libavif_FRAMEWORK_DIRS_DEBUG )
set(libavif_FRAMEWORKS_DEBUG )
set(libavif_BUILD_DIRS_DEBUG )
set(libavif_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(libavif_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${libavif_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${libavif_COMPILE_OPTIONS_C_DEBUG}>")
set(libavif_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libavif_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libavif_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libavif_EXE_LINK_FLAGS_DEBUG}>")


set(libavif_COMPONENTS_DEBUG )