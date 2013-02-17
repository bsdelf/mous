cmake_minimum_required(VERSION 2.8)

project(MOUS)

#=== Compile Flags ====#
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
#set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall")
#set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")
#add_definitions(-Wall)

#==== Library path ====#
include_directories(SYSTEM /usr/local/include)
include_directories(SYSTEM /opt/local/include)
link_directories(/usr/local/lib)
link_directories(/opt/local/lib)
include_directories(BEFORE sdk)
include_directories(BEFORE contrib)

SET(CMAKE_INSTALL_RPATH "\${CMAKE_INSTALL_PREFIX}/lib:\${CMAKE_INSTALL_PREFIX}/bin")

#==== OS Check ====#
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(IS_FREEBSD true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(IS_LINUX true)
elseif(APPLE)
    set(IS_APPLE true)
endif()

#==== amd64/x86 ====#
set(Bitness 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(Bitness 64)
endif()

#==== Compiler ====#
option(UseCompileClang "Use Clang C compiler" off)
option(UseCompileGCC48 "Use GCC48 compiler" off)
if(UseCompileClang AND (NOT UseCompileGCC48))
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
elseif(UseCompileGCC48 AND (NOT UseCompileClang))
    set(CMAKE_C_COMPILER "gcc48")
    set(CMAKE_CXX_COMPILER "g++48")
else()
    message(SEND_ERROR "Use clang or gcc?!!!!")
    return()
endif()

set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install path prefix")
add_definitions(-DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

#==== Frontend ====#
option(WithFrontendQt "Build Qt frontend" off)
option(WithFrontendNcurses "Build ncurses frontend" off)

#==== Optional ====#
option(WithAssembly "Enable Assembly Optimize" on)
option(WithHeadFiles "Install with head files" off)

option(WithPluginOssRenderer "OSS Output" off)
option(WithPluginAlsaRenderer "ALSA output" off)
option(WithPluginAoRenderer "AO output" off)

option(WithPluginWavDecoder "WAV Decoder" on)
option(WithPluginWavEncoder "WAV Encoder" on)
option(WithPluginWmaDecoder "WMA Decoder" off)
option(WithPluginMacDecoder "MAC Decoder" off)
option(WithPluginMac4Decoder "MAC4.11 Decoder" off)
option(WithPluginMpg123Decoder "MPG123 Decoder" off)
option(WithPluginOggDecoder "Ogg Decoder" off)
option(WithPluginOggEncoder "Ogg Encoder" off)
option(WithPluginLameEncoder "LAME Encoder" off)
option(WithPluginFlacDecoder "FLAC Decoder" off)
option(WithPluginFlacEncoder "FLAC Encoder" off)
option(WithPluginFaadDecoder "FAAD Decoder" off)
option(WithPluginFaacEncoder "FAAC Encoder" off)
option(WithPluginWvDecoder "WavPack Decoder" off)

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
if(IS_FREEBSD OR IS_APPLE)
    target_link_libraries(MousCore -pthread)
elseif(IS_LINUX)
    target_link_libraries(MousCore -pthread -ldl)
endif()

# mous-cli
file(GLOB_RECURSE EXE_MOUS_CLI frontend/cli/*)
add_executable(mous-cli ${EXE_MOUS_CLI})
add_dependencies(mous-cli MousCore)
if(IS_FREEBSD OR IS_APPLE)
    target_link_libraries(mous-cli MousCore)
elseif(IS_LINUX)
    target_link_libraries(mous-cli MousCore -ldl)
endif()

# mous-ncurses
if(WithFrontendNcurses)
    file(GLOB EXE_MOUS_NCURSES frontend/ncurses/*)
    add_executable(mous-ncurses ${EXE_MOUS_NCURSES})
    add_dependencies(mous-ncurses MousCore)
    set(EXE_MOUS_NCURSES_LIBS MousCore -pthread -lncursesw)
    if(IS_FREEBSD OR IS_APPLE)
        target_link_libraries(mous-ncurses ${EXE_MOUS_NCURSES_LIBS} -liconv)
    elseif(IS_LINUX)
        target_link_libraries(mous-ncurses ${EXE_MOUS_NCURSES_LIBS} -ldl -lrt)
    endif()
endif()

# mous-qt
if(WithFrontendQt)
    find_package(Qt4 REQUIRED QtCore QtGui)
    include(${QT_USE_FILE})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    file(GLOB EXE_MOUS_QT_HEADERS frontend/qt/*.h frontend/qt/*.hpp)
    file(GLOB EXE_MOUS_QT_SOURCES frontend/qt/*.cpp)
    file(GLOB EXE_MOUS_QT_UI frontend/qt/*.ui)
    file(GLOB EXE_MOUS_QT_QRC frontend/qt/*.qrc)
    file(GLOB EXE_MOUS_QT_TS frontend/qt/*.ts)

    set(EXE_MOUS_QT_MOC_HEADERS)
    foreach(header ${EXE_MOUS_QT_HEADERS})
        file(STRINGS "${header}" lines REGEX "Q_OBJECT")
        if(lines)
            list(APPEND EXE_MOUS_QT_MOC_HEADERS "${header}")
        endif()
    endforeach()

    QT4_WRAP_CPP(EXE_MOUS_QT_WCPP ${EXE_MOUS_QT_MOC_HEADERS})
    QT4_WRAP_UI(EXE_MOUS_QT_WUI ${EXE_MOUS_QT_UI})
    QT4_ADD_RESOURCES(EXE_MOUS_QT_RES ${EXE_MOUS_QT_QRC})
    QT4_ADD_TRANSLATION(EXE_MOUS_QT_QM ${EXE_MOUS_QT_TS})
    add_executable(mous-qt ${EXE_MOUS_QT_SOURCES}
        ${EXE_MOUS_QT_WCPP} ${EXE_MOUS_QT_WUI} ${EXE_MOUS_QT_RES} ${EXE_MOUS_QT_QM})
    add_dependencies(mous-qt MousCore)
    set(EXE_MOUS_QT_LIBS MousCore ${QT_LIBRARIES})
    if(IS_FREEBSD OR IS_APPLE)
        target_link_libraries(mous-qt ${EXE_MOUS_QT_LIBS} -liconv)
    elseif(IS_LINUX)
        target_link_libraries(mous-qt ${EXE_MOUS_QT_LIBS})
    endif()
endif()

# OSS plugin
if(WithPluginOssRenderer)
    file(GLOB SRC_OSS_RENDERER plugins/OssRenderer/*)
    add_library(OssRenderer SHARED ${SRC_OSS_RENDERER})
    target_link_libraries(OssRenderer)
endif()

# ALSA plugin
if(WithPluginAlsaRenderer)
    file(GLOB SRC_ALSA_RENDERER plugins/AlsaRenderer/*)
    add_library(AlsaRenderer SHARED ${SRC_ALSA_RENDERER})
    target_link_libraries(AlsaRenderer -lasound)
endif()

# AO plugin
if(WithPluginAoRenderer)
    file(GLOB SRC_AO_RENDERER plugins/AoRenderer/*)
    add_library(AoRenderer SHARED ${SRC_AO_RENDERER})
    target_link_libraries(AoRenderer -lao)
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

# Ogg plugin
if(WithPluginOggDecoder)
    file(GLOB SRC_OGG_DECODER plugins/OggDecoder/*)
    add_library(OggDecoder SHARED ${SRC_OGG_DECODER})
    target_link_libraries(OggDecoder -lvorbisfile)
endif()
if(WithPluginOggEncoder)
    file(GLOB SRC_OGG_ENCODER plugins/OggEncoder/*)
    add_library(OggEncoder SHARED ${SRC_OGG_ENCODER})
    target_link_libraries(OggEncoder -lvorbisfile)
endif()

# Wma plugin
if(WithPluginWmaDecoder)
    file(GLOB SRC_FFWMA contrib/ffmpeg-strip-wma/*.c)
    file(GLOB SRC_WMA_DECODER plugins/WmaDecoder/*)
    add_library(WmaDecoder SHARED ${SRC_FFWMA} ${SRC_WMA_DECODER})
    set_target_properties(WmaDecoder PROPERTIES 
        COMPILE_FLAGS "-fno-strict-aliasing -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE")
    target_link_libraries(WmaDecoder)
endif()

# Mac plugin
if(WithPluginMacDecoder)
    file(GLOB SRC_MAC_DECODER plugins/MacDecoder/*)
    add_library(MacDecoder SHARED ${SRC_MAC_DECODER})
    target_link_libraries(MacDecoder -lmac)
endif()

# Mac4 plugin(still buggy)
if(WithPluginMac4Decoder)
    if(WithAssembly)
        enable_language(ASM_NASM)
        set(CMAKE_ASM_NASM_COMPILER "yasm")

        if(Bitness EQUAL 64)
            file(GLOB SRC_MAC4_ASM contrib/MACLib/Source/MACLib/Assembly/x86_64_mmx.asm)
            set(YASM_FLAG " -m amd64")
            set(MAC_FLAG " -DENABLE_ASSEMBLY -DARCH_X86_64")
        else()
            file(GLOB SRC_MAC4_ASM contrib/MACLib/Source/MACLib/Assembly/x86_mmx.asm)
            set(YASM_FLAG " -m x86")
            set(MAC_FLAG " -DENABLE_ASSEMBLY -DARCH_X86")
        endif()

        set_source_files_properties(${SRC_MAC4_ASM} COMPILE_FLAGS "-f elf ${YASM_FLAG}") 
    endif()

    file(GLOB SRC_MAC4_LIB contrib/MACLib/Source/MACLib/*.cpp contrib/MACLib/Source/Shared/*.cpp contrib/MACLib/Source/MACLib/Assembly/*.cpp)
    file(GLOB SRC_MAC4_DECODER plugins/Mac4Decoder/*)

    set_source_files_properties(${SRC_MAC4_DECODER} ${SRC_MAC4_LIB}
        COMPILE_FLAGS "-O2 -fno-strict-aliasing -I${MOUS_SOURCE_DIR}/contrib/MACLib/Source/Shared -DBUILD_CROSS_PLATFORM ${MAC_FLAG}")

    add_library(Mac4Decoder SHARED ${SRC_MAC4_DECODER} ${SRC_MAC4_LIB} ${SRC_MAC4_ASM})
    target_link_libraries(Mac4Decoder)
endif()

# Flac plugin
if(WithPluginFlacDecoder)
    file(GLOB SRC_FLAC_DECODER plugins/FlacDecoder/*)
    add_library(FlacDecoder SHARED ${SRC_FLAC_DECODER})
    target_link_libraries(FlacDecoder -lFLAC)
endif()

# WavPack plugin
if(WithPluginWvDecoder)
    file(GLOB SRC_WV_DECODER plugins/WvDecoder/*)
    add_library(WvDecoder SHARED ${SRC_WV_DECODER})
    target_link_libraries(WvDecoder -lwavpack)
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
    if(IS_FREEBSD OR IS_APPLE)
        target_link_libraries(TagLibParser -ltag -liconv)
    elseif(IS_LINUX)
        target_link_libraries(TagLibParser -ltag)
    endif()
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

install(TARGETS mous-cli RUNTIME DESTINATION bin)

if(WithFrontendNcurses)
    install(TARGETS mous-ncurses
        RUNTIME DESTINATION bin)
endif()

if(WithFrontendQt)
    install(TARGETS mous-qt
        RUNTIME DESTINATION bin)
    install(
        CODE "file(GLOB TRANSLATION_FILES \"${PROJECT_BINARY_DIR}/*.qm\")"
        CODE "file(INSTALL \${TRANSLATION_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mous/qt)"
    )
endif()

if(WithHeadFiles)
    file(GLOB HEAD_FILES "${MOUS_SOURCE_DIR}/sdk/*")
    install(DIRECTORY ${HEAD_FILES} DESTINATION include/mous)
endif()

install(
    CODE "file(GLOB PLUGIN_FILES \"${PROJECT_BINARY_DIR}/plugins/*.so\")"
    CODE "file(INSTALL \${PLUGIN_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/mous)"
)

file(GLOB RESOURCE_FILES "${MOUS_SOURCE_DIR}/resource/*")
install(DIRECTORY ${RESOURCE_FILES} DESTINATION share/mous)

#==== Messages ====#
message("CMake:		" ${CMAKE_VERSION})
message("OS Name:	" ${CMAKE_SYSTEM_NAME})
message("OS Version:	" ${CMAKE_SYSTEM_VERSION})
message("C Compiler:	" ${CMAKE_C_COMPILER})
message("C++ Compiler:	" ${CMAKE_CXX_COMPILER})
message("Precessor:	" ${CMAKE_SYSTEM_PROCESSOR})
message("Build Type:	" ${CMAKE_BUILD_TYPE})
message("Project Root:	" ${MOUS_SOURCE_DIR})
message("Work Root:	" ${PROJECT_BINARY_DIR})
message("For Arch:      " ${Bitness})
