

# Conan automatically generated toolchain file
# DO NOT EDIT MANUALLY, it will be overwritten

# Avoid including toolchain file several times (bad if appending to variables like
#   CMAKE_CXX_FLAGS. See https://github.com/android/ndk/issues/323
include_guard()

message(STATUS "Using Conan toolchain: ${CMAKE_CURRENT_LIST_FILE}")

if(${CMAKE_VERSION} VERSION_LESS "3.15")
    message(FATAL_ERROR "The 'CMakeToolchain' generator only works with CMake >= 3.15")
endif()










string(APPEND CONAN_CXX_FLAGS " -m64")
string(APPEND CONAN_C_FLAGS " -m64")
string(APPEND CONAN_SHARED_LINKER_FLAGS " -m64")
string(APPEND CONAN_EXE_LINKER_FLAGS " -m64")



message(STATUS "Conan toolchain: C++ Standard 17 with extensions ON")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Extra c, cxx, linkflags and defines


if(DEFINED CONAN_CXX_FLAGS)
  string(APPEND CMAKE_CXX_FLAGS_INIT " ${CONAN_CXX_FLAGS}")
endif()
if(DEFINED CONAN_C_FLAGS)
  string(APPEND CMAKE_C_FLAGS_INIT " ${CONAN_C_FLAGS}")
endif()
if(DEFINED CONAN_SHARED_LINKER_FLAGS)
  string(APPEND CMAKE_SHARED_LINKER_FLAGS_INIT " ${CONAN_SHARED_LINKER_FLAGS}")
endif()
if(DEFINED CONAN_EXE_LINKER_FLAGS)
  string(APPEND CMAKE_EXE_LINKER_FLAGS_INIT " ${CONAN_EXE_LINKER_FLAGS}")
endif()

get_property( _CMAKE_IN_TRY_COMPILE GLOBAL PROPERTY IN_TRY_COMPILE )
if(_CMAKE_IN_TRY_COMPILE)
    message(STATUS "Running toolchain IN_TRY_COMPILE")
    return()
endif()

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# Definition of CMAKE_MODULE_PATH
# the generators folder (where conan generates files, like this toolchain)
list(PREPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})

# Definition of CMAKE_PREFIX_PATH, CMAKE_XXXXX_PATH
# The Conan local "generators" folder, where this toolchain is saved.
list(PREPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR} )
list(PREPEND CMAKE_LIBRARY_PATH "/home/ethan/.conan2/p/b/glfw38e4adfd7f660/p/lib" "/home/ethan/.conan2/p/b/saila8a406b34a54f/p/lib" "/home/ethan/.conan2/p/b/gifli4a707968bb7bf/p/lib" "/home/ethan/.conan2/p/b/libpnb4a34b12689a6/p/lib" "/home/ethan/.conan2/p/b/libti0c773037b7496/p/lib" "/home/ethan/.conan2/p/b/libde8410e06a517f3/p/lib" "/home/ethan/.conan2/p/b/xz_utdd3b419fb767f/p/lib" "/home/ethan/.conan2/p/b/jbig939f94e0fce62/p/lib" "/home/ethan/.conan2/p/b/zstd3e65d57b8617a/p/lib" "/home/ethan/.conan2/p/b/libav993a76bd6686f/p/lib" "/home/ethan/.conan2/p/b/libao1827feb39a69a/p/lib" "/home/ethan/.conan2/p/b/libyu9c38ea4aea72b/p/lib" "/home/ethan/.conan2/p/b/dav1dd595146e9267f/p/lib" "/home/ethan/.conan2/p/b/jaspebbbc83114db51/p/lib" "/home/ethan/.conan2/p/b/libjpd45ed60092bf2/p/lib" "/home/ethan/.conan2/p/b/libweff8d1a147b0a0/p/lib" "/home/ethan/.conan2/p/b/assim2769463910e93/p/lib" "/home/ethan/.conan2/p/b/miniz5cc6ba3c9f94a/p/lib" "/home/ethan/.conan2/p/b/bzip2a5e3acbdd2d81/p/lib" "/home/ethan/.conan2/p/b/pugix39902bc646c63/p/lib" "/home/ethan/.conan2/p/b/kuba-f5d380cd87da9/p/lib" "/home/ethan/.conan2/p/b/poly284ef6a7914d6d/p/lib" "/home/ethan/.conan2/p/rapid4cabb31a09329/p/lib" "/home/ethan/.conan2/p/b/zliba01937894e7ae/p/lib" "/home/ethan/.conan2/p/b/draco1a42ad98cbe6e/p/lib" "/home/ethan/.conan2/p/b/clippf663f0b5fd8e1/p/lib" "/home/ethan/.conan2/p/b/opendf7f7c6b71b7d0/p/lib")
list(PREPEND CMAKE_INCLUDE_PATH "/home/ethan/.conan2/p/glm67cb345896424/p/include" "/home/ethan/.conan2/p/b/glfw38e4adfd7f660/p/include" "/usr/include/uuid" "/home/ethan/.conan2/p/b/saila8a406b34a54f/p/include" "/home/ethan/.conan2/p/b/saila8a406b34a54f/p/include/sail" "/home/ethan/.conan2/p/b/gifli4a707968bb7bf/p/include" "/home/ethan/.conan2/p/b/libpnb4a34b12689a6/p/include" "/home/ethan/.conan2/p/b/libti0c773037b7496/p/include" "/home/ethan/.conan2/p/b/libde8410e06a517f3/p/include" "/home/ethan/.conan2/p/b/xz_utdd3b419fb767f/p/include" "/home/ethan/.conan2/p/b/jbig939f94e0fce62/p/include" "/home/ethan/.conan2/p/b/zstd3e65d57b8617a/p/include" "/home/ethan/.conan2/p/b/libav993a76bd6686f/p/include" "/home/ethan/.conan2/p/b/libao1827feb39a69a/p/include" "/home/ethan/.conan2/p/b/libyu9c38ea4aea72b/p/include" "/home/ethan/.conan2/p/b/dav1dd595146e9267f/p/include" "/home/ethan/.conan2/p/b/jaspebbbc83114db51/p/include" "/home/ethan/.conan2/p/b/libjpd45ed60092bf2/p/include" "/home/ethan/.conan2/p/b/libweff8d1a147b0a0/p/include" "/home/ethan/.conan2/p/b/assim2769463910e93/p/include" "/home/ethan/.conan2/p/b/miniz5cc6ba3c9f94a/p/include" "/home/ethan/.conan2/p/b/miniz5cc6ba3c9f94a/p/include/minizip" "/home/ethan/.conan2/p/b/bzip2a5e3acbdd2d81/p/include" "/home/ethan/.conan2/p/utfcp9788b2736e129/p/include" "/home/ethan/.conan2/p/utfcp9788b2736e129/p/include/utf8cpp" "/home/ethan/.conan2/p/b/pugix39902bc646c63/p/include" "/home/ethan/.conan2/p/b/kuba-f5d380cd87da9/p/include" "/home/ethan/.conan2/p/b/poly284ef6a7914d6d/p/include" "/home/ethan/.conan2/p/rapid4cabb31a09329/p/include" "/home/ethan/.conan2/p/b/zliba01937894e7ae/p/include" "/home/ethan/.conan2/p/b/draco1a42ad98cbe6e/p/include" "/home/ethan/.conan2/p/b/clippf663f0b5fd8e1/p/include" "/home/ethan/.conan2/p/stb974d150741cda/p/include" "/home/ethan/.conan2/p/b/opendf7f7c6b71b7d0/p/include")



if (DEFINED ENV{PKG_CONFIG_PATH})
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}:$ENV{PKG_CONFIG_PATH}")
else()
set(ENV{PKG_CONFIG_PATH} "${CMAKE_CURRENT_LIST_DIR}:")
endif()




# Variables
# Variables  per configuration


# Preprocessor definitions
# Preprocessor definitions per configuration
