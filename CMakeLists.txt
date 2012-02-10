cmake_minimum_required(VERSION 2.8)

project(MOUS)

# compile flags
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O2 -Wall")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -g -Wall")

# os check
set(IS_FREEBSD "${CMAKE_SYSTEM}" MATCHES "FreeBSD")
set(IS_LINUX "${CMAKE_SYSTEM}" MATCHES "Linux")
set(IS_APPLE "APPLE")

# options
option(EnableClang "Use Clang C compiler" off)
if (EnableClang)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

option(EnableGCC46 "Use GCC46 compiler" off)
if (EnableGCC46)
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
target_link_libraries(MousCore -pthread)

# mous-cli
file(GLOB_RECURSE EXE_MOUS_CLI cli/*.cpp)
add_executable(mous-cli ${EXE_MOUS_CLI})
target_link_libraries(mous-cli MousCore)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/plugins)

# mp3 plugin
file(GLOB SRC_MP3_DECODER plugins/mp3decoder/*.cpp)
add_library(Mp3Decoder SHARED ${SRC_MP3_DECODER})
target_link_libraries(Mp3Decoder -lmpg123)

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
