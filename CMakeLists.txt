cmake_minimum_required(VERSION 2.8)

project(MOUS)

# compile flags
set(CMAKE_C_FLAGS "$ENV{CFLAGS} -O2 -Wall")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O2 -Wall")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -g -Wall")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -O2 -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g -Wall")

# os check
if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(IS_FREEBSD true)
elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(IS_LINUX true)
endif()
#set(IS_APPLE "APPLE")

# options
option(EnableCompileClang "Use Clang C compiler" off)
option(EnableCompileGCC46 "Use GCC46 compiler" off)

option(EnablePluginOssRenderer "OSS output" on)
option(EnablePluginApeDecoder "APE Decoder" on)
option(EnablePluginMp3Decoder "MP3 Decoder" on)
option(EnablePluginFlacDecoder "FLAC Decoder" on)
option(EnablePluginAacDecoder "AAC Decoder" on)

if (EnableCompileClang)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

if (EnableCompileGCC46)
    set(CMAKE_C_COMPILER "gcc46")
    set(CMAKE_CXX_COMPILER "g++46")
endif()

# include and link
include_directories(SYSTEM /usr/local/include)
link_directories(/usr/local/lib)
include_directories(BEFORE sdk core)

# build output dir
set(OUTPUT_DIR ${MOUS_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# libMousCore
file(GLOB_RECURSE LIB_MOUS_CORE core/*.cpp)
add_library(MousCore SHARED ${LIB_MOUS_CORE})
if (IS_FREEBSD)
    target_link_libraries(MousCore -pthread)
elseif (IS_LINUX)
    target_link_libraries(MousCore -pthread -ldl)
else()
    target_link_libraries(MousCore -pthread)
endif()

# mous-cli
file(GLOB_RECURSE EXE_MOUS_CLI cli/*.cpp)
add_executable(mous-cli ${EXE_MOUS_CLI})
if (IS_FREEBSD)
    target_link_libraries(mous-cli MousCore)
elseif (IS_LINUX)
    target_link_libraries(mous-cli MousCore -ldl)
else()
    target_link_libraries(mous-cli MousCore)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/plugins)

# oss plugin
if (EnablePluginOssRenderer)
    file(GLOB SRC_OSS_RENDERER plugins/oss/*.cpp)
    add_library(OssRenderer SHARED ${SRC_OSS_RENDERER})
    target_link_libraries(OssRenderer)
endif()

# mp3 plugin
if (EnablePluginMp3Decoder)
    file(GLOB SRC_MP3_DECODER plugins/mp3decoder/*.cpp)
    add_library(Mp3Decoder SHARED ${SRC_MP3_DECODER})
    target_link_libraries(Mp3Decoder -lmpg123)
endif()

# ape plugin
if (EnablePluginApeDecoder)
    file(GLOB SRC_APE_DECODER plugins/apedecoder/*.cpp)
    add_library(ApeDecoder SHARED ${SRC_APE_DECODER})
    target_link_libraries(ApeDecoder -lmac)
endif()

# flac plugin
if (EnablePluginFlacDecoder)
    file(GLOB SRC_FLAC_DECODER plugins/flacdecoder/*.cpp)
    add_library(FlacDecoder SHARED ${SRC_FLAC_DECODER})
    target_link_libraries(FlacDecoder -lFLAC)
endif()

# aac plugin
if (EnablePluginAacDecoder)
    file(GLOB SRC_FAAD_DECODER plugins/m4adecoder/*)
    add_library(FaadDecoder SHARED ${SRC_FAAD_DECODER})
    target_link_libraries(FaadDecoder -lfaad -lmp4ff)
endif()

# messages
message("CMake:		" ${CMAKE_VERSION})
message("OS Name:	" ${CMAKE_SYSTEM_NAME})
message("OS Version:	" ${CMAKE_SYSTEM_VERSION})
message("C Compiler:	" ${CMAKE_C_COMPILER})
message("C++ Compiler:	" ${CMAKE_CXX_COMPILER})
message("Precessor:	" ${CMAKE_SYSTEM_PROCESSOR})
message("Build type:	" ${CMAKE_BUILD_TYPE})
message("Project root:	" ${MOUS_SOURCE_DIR})
message("Ouput root:	" ${OUTPUT_DIR})
