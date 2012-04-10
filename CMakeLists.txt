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

#==== Compiler ====#
option(EnableCompileClang "Use Clang C compiler" off)
option(EnableCompileGCC46 "Use GCC46 compiler" off)

if (EnableCompileClang)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

if (EnableCompileGCC46)
    set(CMAKE_C_COMPILER "gcc46")
    set(CMAKE_CXX_COMPILER "g++46")
endif()

#==== Optional ====#
option(EnablePluginOssRenderer "OSS Output" on)
option(EnablePluginAoRenderer "AO output" on)
option(EnablePluginAlsaRenderer "ALSA output" off)

option(EnablePluginWavDecoder "WAV Decoder" on)
option(EnablePluginWavEncoder "WAV Encoder" on)
option(EnablePluginMacDecoder "MAC Decoder" on)
option(EnablePluginMpg123Decoder "MPG123 Decoder" on)
option(EnablePluginLameEncoder "LAME Encoder" on)
option(EnablePluginFlacDecoder "FLAC Decoder" on)
option(EnablePluginFlacEncoder "FLAC Encoder" on)
option(EnablePluginFaadDecoder "FAAD Decoder" on)
option(EnablePluginFaacEncoder "FAAC Encoder" on)

option(EnablePluginTagLibParser "TagLib Parser" on)
option(EnablePluginCuePack "Cue sheet Parser" on)

option(EnablePluginPlayerStatistics "Player activity statistics" on)

#==== Library path ====#
# include and link
include_directories(SYSTEM /usr/local/include)
link_directories(/usr/local/lib)
include_directories(BEFORE sdk)

#==== build output dir ====#
set(OUTPUT_DIR ${MOUS_SOURCE_DIR}/build)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})

# libCharsetConv
file(GLOB_RECURSE LIB_CHARSET_CONV charset/*.cpp charset/uchardect/src/*)
add_library(CharsetConv SHARED ${LIB_CHARSET_CONV})
if (NOT IS_LINUX)
    target_link_libraries(CharsetConv -liconv)
endif()

# libMousCore
file(GLOB_RECURSE LIB_MOUS_CORE core/*)
add_library(MousCore SHARED ${LIB_MOUS_CORE})
if (IS_FREEBSD)
    target_link_libraries(MousCore -pthread)
elseif (IS_LINUX)
    target_link_libraries(MousCore -pthread -ldl)
else()
    target_link_libraries(MousCore -pthread)
endif()

# mous-cli
file(GLOB_RECURSE EXE_MOUS_CLI mous-cli/*)
add_executable(mous-cli ${EXE_MOUS_CLI})
if (IS_FREEBSD)
    target_link_libraries(mous-cli MousCore CharsetConv)
elseif (IS_LINUX)
    target_link_libraries(mous-cli MousCore CharsetConv -ldl)
else()
    target_link_libraries(mous-cli MousCore CharsetConv)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/plugins)

# OSS plugin
if (EnablePluginOssRenderer)
    file(GLOB SRC_OSS_RENDERER plugins/OssRenderer/*)
    add_library(OssRenderer SHARED ${SRC_OSS_RENDERER})
    target_link_libraries(OssRenderer)
endif()

# AO plugin
if (EnablePluginAoRenderer)
    file(GLOB SRC_AO_RENDERER plugins/AoRenderer/*)
    add_library(AoRenderer SHARED ${SRC_AO_RENDERER})
    target_link_libraries(AoRenderer -lao)
endif()

# ALSA plugin
if (EnablePluginAlsaRenderer)
    file(GLOB SRC_ALSA_RENDERER plugins/AlsaRenderer/*)
    add_library(AlsaRenderer SHARED ${SRC_ALSA_RENDERER})
    target_link_libraries(AlsaRenderer -lasound)
endif()

# WAV plugin
if (EnablePluginWavDecoder)
    file(GLOB SRC_WAV_DECODER plugins/WavDecoder/*)
    add_library(WavDecoder SHARED ${SRC_WAV_DECODER})
endif()
if (EnablePluginWavEncoder)
    file(GLOB SRC_WAV_ENCODER plugins/WavEncoder/*)
    add_library(WavEncoder SHARED ${SRC_WAV_ENCODER})
endif()

# MPG123 & Lame plugin
if (EnablePluginMpg123Decoder)
    file(GLOB SRC_MPG123_DECODER plugins/Mpg123Decoder/*)
    add_library(Mpg123Decoder SHARED ${SRC_MPG123_DECODER})
    target_link_libraries(Mpg123Decoder -lmpg123)
endif()
if (EnablePluginLameEncoder)
    file(GLOB SRC_LAME_ENCODER plugins/LameEncoder/*)
    add_library(LameEncoder SHARED ${SRC_LAME_ENCODER})
    target_link_libraries(LameEncoder -lmp3lame)
endif()

# Mac plugin
if (EnablePluginMacDecoder)
    file(GLOB SRC_MAC_DECODER plugins/MacDecoder/*)
    add_library(MacDecoder SHARED ${SRC_MAC_DECODER})
    target_link_libraries(MacDecoder -lmac)
endif()

# Flac plugin
if (EnablePluginFlacDecoder)
    file(GLOB SRC_FLAC_DECODER plugins/FlacDecoder/*)
    add_library(FlacDecoder SHARED ${SRC_FLAC_DECODER})
    target_link_libraries(FlacDecoder -lFLAC)
endif()

# FAAD & FAAC plugin
if (EnablePluginFaadDecoder)
    file(GLOB SRC_FAAD_DECODER plugins/FaadDecoder/*)
    add_library(FaadDecoder SHARED ${SRC_FAAD_DECODER})
    target_link_libraries(FaadDecoder -lfaad -lmp4ff)
endif()
if (EnablePluginFaacEncoder)
    file(GLOB SRC_FAAC_ENCODER plugins/FaacEncoder/*)
    add_library(FaacEncoder SHARED ${SRC_FAAC_ENCODER})
    target_link_libraries(FaacEncoder -lfaac -lmp4v2)
endif()

# TagLib plugin
if (EnablePluginTagLibParser)
    file(GLOB SRC_TAGLIB_PARSER plugins/TagLibParser/*)
    add_library(TagLibParser SHARED ${SRC_TAGLIB_PARSER})
    target_link_libraries(TagLibParser -ltag)
endif()

# CuePack plugin
if (EnablePluginCuePack)
    file(GLOB SRC_CUEPACK plugins/CuePack/*)
    add_library(CuePack SHARED ${SRC_CUEPACK})
    target_link_libraries(CuePack -lcue)
endif()

# PlayerStatistics plugin
if (EnablePluginPlayerStatistics)
    file(GLOB SRC_PLAYERSTATISTICS plugins/PlayerStatistics/*)
    add_library(PlayerStatistics SHARED ${SRC_PLAYERSTATISTICS})
    target_link_libraries(PlayerStatistics)
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
