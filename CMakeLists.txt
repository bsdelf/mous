cmake_minimum_required(VERSION 2.8)

project(MOUS)

#=== Compile Flags ====#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")

#==== Library path ====#
include_directories(SYSTEM /usr/local/include)
include_directories(SYSTEM /opt/local/include)
link_directories(/usr/local/lib)
link_directories(/opt/local/lib)
include_directories(BEFORE sdk)
include_directories(BEFORE contrib)

#==== OS Check ====#
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(IS_FREEBSD true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(IS_LINUX true)
elseif(APPLE)
    set(IS_APPLE true)
endif()

#==== Compiler ====#
option(UseCompileClang "Use Clang C compiler" off)

if(UseCompileClang)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
endif()

#==== Frontend ====#
option(WithFrontendQt "Build Qt frontend" off)
option(WithFrontendNcurses "Build ncurses frontend" off)

#==== Optional ====#
option(WithHeadFiles "Install with head files" off)

option(WithPluginOssRenderer "OSS Output" off)
option(WithPluginAlsaRenderer "ALSA output" off)
option(WithPluginAoRenderer "AO output" off)

option(WithPluginWavDecoder "WAV Decoder" on)
option(WithPluginWavEncoder "WAV Encoder" on)
option(WithPluginMacDecoder "MAC Decoder" off)
option(WithPluginMac4Decoder "MAC4.x Decoder" off)
option(WithPluginMpg123Decoder "MPG123 Decoder" off)
option(WithPluginLameEncoder "LAME Encoder" off)
option(WithPluginFlacDecoder "FLAC Decoder" off)
option(WithPluginFlacEncoder "FLAC Encoder" off)
option(WithPluginFaadDecoder "FAAD Decoder" off)
option(WithPluginFaacEncoder "FAAC Encoder" off)

option(WithPluginTagLibParser "TagLib Parser" on)
option(WithPluginCuePack "Cue sheet Parser" on)

#option(WithPluginPlayerStatistics "Player activity statistics" on)

#==== Output Dir ====#
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins)

# libCharsetConv
#file(GLOB_RECURSE LIB_CHARSET_CONV charset/*.cpp charset/uchardect/src/*)
#add_library(CharsetConv SHARED ${LIB_CHARSET_CONV})
#if(NOT IS_LINUX)
#    target_link_libraries(CharsetConv -liconv)
#endif()

#==== Targets ====#

# libMousCore
file(GLOB_RECURSE LIB_MOUS_CORE core/*)
add_library(MousCore SHARED ${LIB_MOUS_CORE})
set_target_properties(MousCore PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
if(IS_FREEBSD)
    target_link_libraries(MousCore -pthread)
elseif(IS_LINUX)
    target_link_libraries(MousCore -pthread -ldl)
else()
    target_link_libraries(MousCore -pthread)
endif()

# mous-cli
file(GLOB_RECURSE EXE_MOUS_CLI mous-cli/*)
add_executable(mous-cli ${EXE_MOUS_CLI})
add_dependencies(mous-cli MousCore)
if(IS_FREEBSD)
    target_link_libraries(mous-cli MousCore)
elseif(IS_LINUX)
    target_link_libraries(mous-cli MousCore -ldl)
else()
    target_link_libraries(mous-cli MousCore)
endif()

# mous-ncurses
if(WithFrontendNcurses)
    file(GLOB_RECURSE EXE_MOUS_NCURSES mous-ncurses/*)
    add_executable(mous-ncurses ${EXE_MOUS_NCURSES})
    add_dependencies(mous-ncurses MousCore)
    if(IS_FREEBSD)
        target_link_libraries(mous-ncurses MousCore -pthread -lncursesw -liconv)
    elseif(IS_LINUX)
        target_link_libraries(mous-ncurses MousCore -pthread -lncursesw -ldl)
    else()
        target_link_libraries(mous-ncurses MousCore -pthread -lncursesw -liconv)
    endif()
endif()

# mous-qt
if(WithFrontendQt)
    find_package(Qt4 REQUIRED QtCore QtGui)
    include(${QT_USE_FILE})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    file(GLOB EXE_MOUS_QT_HEADS mous-qt/*.h mous-qt/*.hpp)
    file(GLOB EXE_MOUS_QT_SOURCES mous-qt/*.cpp)
    file(GLOB EXE_MOUS_QT_UI mous-qt/*.ui)
    file(GLOB EXE_MOUS_QT_QRC mous-qt/*.qrc)
    QT4_WRAP_CPP(EXE_MOUS_QT_WCPP ${EXE_MOUS_QT_HEADS})
    QT4_WRAP_UI(EXE_MOUS_QT_WUI ${EXE_MOUS_QT_UI})
    QT4_ADD_RESOURCES(EXE_MOUS_QT_WRES ${EXE_MOUS_QT_QRC})
    add_executable(qmous ${EXE_MOUS_QT_SOURCES}
        ${EXE_MOUS_QT_WCPP} ${EXE_MOUS_QT_WUI} ${EXE_MOUS_QT_WRES})
    add_dependencies(qmous MousCore)
    target_link_libraries(qmous MousCore -liconv ${QT_LIBRARIES})
endif()

# OSS plugin
if(WithPluginOssRenderer)
    file(GLOB SRC_OSS_RENDERER plugins/OssRenderer/*)
    add_library(OssRenderer SHARED ${SRC_OSS_RENDERER})
    target_link_libraries(OssRenderer)
endif()

# AO plugin
if(WithPluginAoRenderer)
    file(GLOB SRC_AO_RENDERER plugins/AoRenderer/*)
    add_library(AoRenderer SHARED ${SRC_AO_RENDERER})
    target_link_libraries(AoRenderer -lao)
endif()

# ALSA plugin
if(WithPluginAlsaRenderer)
    file(GLOB SRC_ALSA_RENDERER plugins/AlsaRenderer/*)
    add_library(AlsaRenderer SHARED ${SRC_ALSA_RENDERER})
    target_link_libraries(AlsaRenderer -lasound)
endif()

# WAV plugin
if(WithPluginWavDecoder)
    file(GLOB SRC_WAV_DECODER plugins/WavDecoder/*)
    add_library(WavDecoder SHARED ${SRC_WAV_DECODER})
endif()
if(WithPluginWavEncoder)
    file(GLOB SRC_WAV_ENCODER plugins/WavEncoder/*)
    add_library(WavEncoder SHARED ${SRC_WAV_ENCODER})
endif()

# MPG123 & Lame plugin
if(WithPluginMpg123Decoder)
    file(GLOB SRC_MPG123_DECODER plugins/Mpg123Decoder/*)
    add_library(Mpg123Decoder SHARED ${SRC_MPG123_DECODER})
    target_link_libraries(Mpg123Decoder -lmpg123)
endif()
if(WithPluginLameEncoder)
    file(GLOB SRC_LAME_ENCODER plugins/LameEncoder/*)
    add_library(LameEncoder SHARED ${SRC_LAME_ENCODER})
    target_link_libraries(LameEncoder -lmp3lame)
endif()

# Mac plugin
if(WithPluginMacDecoder)
    file(GLOB SRC_MAC_DECODER plugins/MacDecoder/*)
    add_library(MacDecoder SHARED ${SRC_MAC_DECODER})
    target_link_libraries(MacDecoder -lmac)
endif()

# Mac4 plugin
if(WithPluginMac4Decoder)
    file(GLOB SRC_MAC4_DECODER plugins/Mac4Decoder/* contrib/MACLib/*)
    add_library(Mac4Decoder SHARED ${SRC_MAC4_DECODER})
endif()

# Flac plugin
if(WithPluginFlacDecoder)
    file(GLOB SRC_FLAC_DECODER plugins/FlacDecoder/*)
    add_library(FlacDecoder SHARED ${SRC_FLAC_DECODER})
    target_link_libraries(FlacDecoder -lFLAC)
endif()

# FAAD & FAAC plugin
if(WithPluginFaadDecoder)
    file(GLOB SRC_FAAD_DECODER plugins/FaadDecoder/*)
    add_library(FaadDecoder SHARED ${SRC_FAAD_DECODER})
    target_link_libraries(FaadDecoder -lfaad -lmp4ff)
endif()
if(WithPluginFaacEncoder)
    file(GLOB SRC_FAAC_ENCODER plugins/FaacEncoder/*)
    add_library(FaacEncoder SHARED ${SRC_FAAC_ENCODER})
    target_link_libraries(FaacEncoder -lfaac -lmp4v2)
endif()

# TagLib plugin
if(WithPluginTagLibParser)
    file(GLOB SRC_TAGLIB_PARSER plugins/TagLibParser/*)
    add_library(TagLibParser SHARED ${SRC_TAGLIB_PARSER})
    target_link_libraries(TagLibParser -ltag)
endif()

# CuePack plugin
if(WithPluginCuePack)
    file(GLOB SRC_CUEPACK plugins/CuePack/*)
    add_library(CuePack SHARED ${SRC_CUEPACK})
    target_link_libraries(CuePack -lcue)
endif()

# PlayerStatistics plugin
#if(WithPluginPlayerStatistics)
#    file(GLOB SRC_PLAYERSTATISTICS plugins/PlayerStatistics/*)
#    add_library(PlayerStatistics SHARED ${SRC_PLAYERSTATISTICS})
#    target_link_libraries(PlayerStatistics)
#endif()

#==== Install Targets ====#
install(TARGETS MousCore
        LIBRARY DESTINATION lib)

if(WithFrontendNcurses)
    install(TARGETS mous-ncurses
        RUNTIME DESTINATION bin)
endif()

if(WithFrontendQt)
    install(TARGETS qmous
        RUNTIME DESTINATION bin)
endif()

if(WithHeadFiles)
    file(GLOB HEAD_FILES "${MOUS_SOURCE_DIR}/sdk/*")
    install(DIRECTORY ${HEAD_FILES} DESTINATION include/mous)
endif()

file(GLOB PLUGINS_LIBS "${PROJECT_BINARY_DIR}/plugins/*.so")
install(FILES ${PLUGINS_LIBS} DESTINATION lib/mous)

file(GLOB RESOURCE_FILES "${MOUS_SOURCE_DIR}/resource/*")
install(DIRECTORY ${RESOURCE_FILES} DESTINATION share/mous)

#==== Messages ====#
message("CMake:		" ${CMAKE_VERSION})
message("OS Name:	" ${CMAKE_SYSTEM_NAME})
message("OS Version:	" ${CMAKE_SYSTEM_VERSION})
message("C Compiler:	" ${CMAKE_C_COMPILER})
message("C++ Compiler:	" ${CMAKE_CXX_COMPILER})
message("Precessor:	" ${CMAKE_SYSTEM_PROCESSOR})
message("Build type:	" ${CMAKE_BUILD_TYPE})
message("Project root:	" ${MOUS_SOURCE_DIR})
message("Work root:	" ${PROJECT_BINARY_DIR})
