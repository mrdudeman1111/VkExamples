########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(libaom-av1_COMPONENT_NAMES "")
set(libaom-av1_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(libaom-av1_PACKAGE_FOLDER_DEBUG "/home/ethan/.conan2/p/b/libao1827feb39a69a/p")
set(libaom-av1_BUILD_MODULES_PATHS_DEBUG )


set(libaom-av1_INCLUDE_DIRS_DEBUG )
set(libaom-av1_RES_DIRS_DEBUG )
set(libaom-av1_DEFINITIONS_DEBUG )
set(libaom-av1_SHARED_LINK_FLAGS_DEBUG )
set(libaom-av1_EXE_LINK_FLAGS_DEBUG )
set(libaom-av1_OBJECTS_DEBUG )
set(libaom-av1_COMPILE_DEFINITIONS_DEBUG )
set(libaom-av1_COMPILE_OPTIONS_C_DEBUG )
set(libaom-av1_COMPILE_OPTIONS_CXX_DEBUG )
set(libaom-av1_LIB_DIRS_DEBUG "${libaom-av1_PACKAGE_FOLDER_DEBUG}/lib")
set(libaom-av1_BIN_DIRS_DEBUG )
set(libaom-av1_LIBRARY_TYPE_DEBUG STATIC)
set(libaom-av1_IS_HOST_WINDOWS_DEBUG 0)
set(libaom-av1_LIBS_DEBUG aom)
set(libaom-av1_SYSTEM_LIBS_DEBUG pthread m)
set(libaom-av1_FRAMEWORK_DIRS_DEBUG )
set(libaom-av1_FRAMEWORKS_DEBUG )
set(libaom-av1_BUILD_DIRS_DEBUG )
set(libaom-av1_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(libaom-av1_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${libaom-av1_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${libaom-av1_COMPILE_OPTIONS_C_DEBUG}>")
set(libaom-av1_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${libaom-av1_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${libaom-av1_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${libaom-av1_EXE_LINK_FLAGS_DEBUG}>")


set(libaom-av1_COMPONENTS_DEBUG )