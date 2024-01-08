########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(jbig_COMPONENT_NAMES "")
set(jbig_FIND_DEPENDENCY_NAMES "")

########### VARIABLES #######################################################################
#############################################################################################
set(jbig_PACKAGE_FOLDER_DEBUG "/home/ethan/.conan2/p/b/jbig939f94e0fce62/p")
set(jbig_BUILD_MODULES_PATHS_DEBUG )


set(jbig_INCLUDE_DIRS_DEBUG )
set(jbig_RES_DIRS_DEBUG )
set(jbig_DEFINITIONS_DEBUG )
set(jbig_SHARED_LINK_FLAGS_DEBUG )
set(jbig_EXE_LINK_FLAGS_DEBUG )
set(jbig_OBJECTS_DEBUG )
set(jbig_COMPILE_DEFINITIONS_DEBUG )
set(jbig_COMPILE_OPTIONS_C_DEBUG )
set(jbig_COMPILE_OPTIONS_CXX_DEBUG )
set(jbig_LIB_DIRS_DEBUG "${jbig_PACKAGE_FOLDER_DEBUG}/lib")
set(jbig_BIN_DIRS_DEBUG )
set(jbig_LIBRARY_TYPE_DEBUG STATIC)
set(jbig_IS_HOST_WINDOWS_DEBUG 0)
set(jbig_LIBS_DEBUG jbig)
set(jbig_SYSTEM_LIBS_DEBUG )
set(jbig_FRAMEWORK_DIRS_DEBUG )
set(jbig_FRAMEWORKS_DEBUG )
set(jbig_BUILD_DIRS_DEBUG )
set(jbig_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(jbig_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${jbig_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${jbig_COMPILE_OPTIONS_C_DEBUG}>")
set(jbig_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${jbig_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${jbig_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${jbig_EXE_LINK_FLAGS_DEBUG}>")


set(jbig_COMPONENTS_DEBUG )