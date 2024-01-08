########### AGGREGATED COMPONENTS AND DEPENDENCIES FOR THE MULTI CONFIG #####################
#############################################################################################

set(jasper_COMPONENT_NAMES "")
list(APPEND jasper_FIND_DEPENDENCY_NAMES JPEG)
list(REMOVE_DUPLICATES jasper_FIND_DEPENDENCY_NAMES)
set(JPEG_FIND_MODE "NO_MODULE")

########### VARIABLES #######################################################################
#############################################################################################
set(jasper_PACKAGE_FOLDER_DEBUG "/home/ethan/.conan2/p/b/jaspebbbc83114db51/p")
set(jasper_BUILD_MODULES_PATHS_DEBUG "${jasper_PACKAGE_FOLDER_DEBUG}/lib/cmake/conan-official-jasper-variables.cmake")


set(jasper_INCLUDE_DIRS_DEBUG )
set(jasper_RES_DIRS_DEBUG )
set(jasper_DEFINITIONS_DEBUG )
set(jasper_SHARED_LINK_FLAGS_DEBUG )
set(jasper_EXE_LINK_FLAGS_DEBUG )
set(jasper_OBJECTS_DEBUG )
set(jasper_COMPILE_DEFINITIONS_DEBUG )
set(jasper_COMPILE_OPTIONS_C_DEBUG )
set(jasper_COMPILE_OPTIONS_CXX_DEBUG )
set(jasper_LIB_DIRS_DEBUG "${jasper_PACKAGE_FOLDER_DEBUG}/lib")
set(jasper_BIN_DIRS_DEBUG )
set(jasper_LIBRARY_TYPE_DEBUG STATIC)
set(jasper_IS_HOST_WINDOWS_DEBUG 0)
set(jasper_LIBS_DEBUG jasper)
set(jasper_SYSTEM_LIBS_DEBUG m pthread)
set(jasper_FRAMEWORK_DIRS_DEBUG )
set(jasper_FRAMEWORKS_DEBUG )
set(jasper_BUILD_DIRS_DEBUG )
set(jasper_NO_SONAME_MODE_DEBUG FALSE)


# COMPOUND VARIABLES
set(jasper_COMPILE_OPTIONS_DEBUG
    "$<$<COMPILE_LANGUAGE:CXX>:${jasper_COMPILE_OPTIONS_CXX_DEBUG}>"
    "$<$<COMPILE_LANGUAGE:C>:${jasper_COMPILE_OPTIONS_C_DEBUG}>")
set(jasper_LINKER_FLAGS_DEBUG
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,SHARED_LIBRARY>:${jasper_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,MODULE_LIBRARY>:${jasper_SHARED_LINK_FLAGS_DEBUG}>"
    "$<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:${jasper_EXE_LINK_FLAGS_DEBUG}>")


set(jasper_COMPONENTS_DEBUG )