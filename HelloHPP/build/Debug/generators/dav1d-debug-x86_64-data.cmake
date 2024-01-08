########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(dav1d_COMPONENT_NAMES "")
set(dav1d_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(dav1d_PACKAGE_FOLDER_DEBUG "/home/ethan/.conan2/p/b/dav1dd595146e9267f/p")
set(dav1d_BUILD_MODULES_PATHS_DEBUG )


set(dav1d_INCLUDE_DIRS_DEBUG )
set(dav1d_RES_DIRS_DEBUG )
set(dav1d_DEFINITIONS_DEBUG )
set(dav1d_SHARED_LINK_FLAGS_DEBUG )
set(dav1d_EXE_LINK_FLAGS_DEBUG )
set(dav1d_OBJECTS_DEBUG )
set(dav1d_COMPILE_DEFINITIONS_DEBUG )
set(dav1d_COMPILE_OPTIONS_C_DEBUG )
set(dav1d_COMPILE_OPTIONS_CXX_DEBUG )
set(dav1d_LIB_DIRS_DEBUG "${dav1d_PACKAGE_FOLDER_DEBUG}/lib")
set(dav1d_BIN_DIRS_DEBUG )
set(dav1d_LIBRARY_TYPE_DEBUG STATIC)
set(dav1d_IS_HOST_WINDOWS_DEBUG 0)
set(dav1d_LIBS_DEBUG dav1d)
set(dav1d_SYSTEM_LIBS_DEBUG dl pthread)
set(dav1d_FRAMEWORK_DIRS_DEBUG )
set(dav1d_FRAMEWORKS_DEBUG )
set(dav1d_BUILD_DIRS_DEBUG )
set(dav1d_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(dav1d_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${dav1d_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${dav1d_COMPILE_OPTIONS_C_DEBUG}>")
set(dav1d_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${dav1d_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${dav1d_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${dav1d_EXE_LINK_FLAGS_DEBUG}>")


set(dav1d_COMPONENTS_DEBUG )