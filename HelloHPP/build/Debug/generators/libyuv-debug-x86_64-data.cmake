########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(libyuv_COMPONENT_NAMES "")
list(APPEND libyuv_FIND_DEPENDENCY_NAMES JPEG)
list(REMOVE_DUPLICATES libyuv_FIND_DEPENDENCY_NAMES)
set(JPEG_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(libyuv_PACKAGE_FOLDER_DEBUG "/home/ethan/.conan2/p/b/libyu9c38ea4aea72b/p")
set(libyuv_BUILD_MODULES_PATHS_DEBUG )


set(libyuv_INCLUDE_DIRS_DEBUG )
set(libyuv_RES_DIRS_DEBUG )
set(libyuv_DEFINITIONS_DEBUG )
set(libyuv_SHARED_LINK_FLAGS_DEBUG )
set(libyuv_EXE_LINK_FLAGS_DEBUG )
set(libyuv_OBJECTS_DEBUG )
set(libyuv_COMPILE_DEFINITIONS_DEBUG )
set(libyuv_COMPILE_OPTIONS_C_DEBUG )
set(libyuv_COMPILE_OPTIONS_CXX_DEBUG )
set(libyuv_LIB_DIRS_DEBUG "${libyuv_PACKAGE_FOLDER_DEBUG}/lib")
set(libyuv_BIN_DIRS_DEBUG )
set(libyuv_LIBRARY_TYPE_DEBUG STATIC)
set(libyuv_IS_HOST_WINDOWS_DEBUG 0)
set(libyuv_LIBS_DEBUG yuv)
set(libyuv_SYSTEM_LIBS_DEBUG m)
set(libyuv_FRAMEWORK_DIRS_DEBUG )
set(libyuv_FRAMEWORKS_DEBUG )
set(libyuv_BUILD_DIRS_DEBUG )
set(libyuv_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(libyuv_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${libyuv_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${libyuv_COMPILE_OPTIONS_C_DEBUG}>")
set(libyuv_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libyuv_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libyuv_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libyuv_EXE_LINK_FLAGS_DEBUG}>")


set(libyuv_COMPONENTS_DEBUG )