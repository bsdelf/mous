cmake_minimum_required(VERSION 3.4)

project(MOUS)

#=== Compiler Settings====#
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#==== Library path ====#
include_directories(SYSTEM /usr/local/include)
link_directories(/usr/local/lib)
include_directories(BEFORE sdk)
include_directories(BEFORE contrib)

#==== threading ====#
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

SET(CMAKE_INSTALL_RPATH "\${CMAKE_INSTALL_PREFIX}/lib:\${CMAKE_INSTALL_PREFIX}/bin")

#==== OS Check ====#
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(IS_FREEBSD true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(IS_LINUX true)
elseif(APPLE)
    set(IS_APPLE true)
endif()

if(NOT IS_APPLE)
    add_definitions(-DUSE_POSIX_SEMAPHORE)
endif()

#==== amd64/x86 ====#
set(Bitness 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(Bitness 64)
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
option(WithPluginMPG123 "MPG123 Decoder" off)
option(WithPluginVorbisDecoder "Ogg Decoder" off)
option(WithPluginVorbisEncoder "Ogg Encoder" off)
option(WithPluginLAME "LAME Encoder" off)
option(WithPluginFlacDecoder "FLAC Decoder" off)
option(WithPluginFlacEncoder "FLAC Encoder" off)
option(WithPluginFdkDecoder "Fdk Decoder" off)
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
    target_link_libraries(MousCore Threads::Threads)
elseif(IS_LINUX)
    target_link_libraries(MousCore Threads::Threads -ldl)
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
    set(EXE_MOUS_NCURSES_LIBS MousCore Threads::Threads -lncursesw)
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
    file(GLOB OSS_RENDERER_SRC plugins/oss/*)
    add_library(OssRenderer SHARED ${OSS_RENDERER_SRC})
    target_link_libraries(OssRenderer)
endif()

# ALSA plugin
if(WithPluginAlsaRenderer)
    file(GLOB ALSA_RENDERER_SRC plugins/alsa/*)
    add_library(AlsaRenderer SHARED ${ALSA_RENDERER_SRC})
    target_link_libraries(AlsaRenderer -lasound)
endif()

# AO plugin
if(WithPluginAoRenderer)
    find_library(LIBAO ao)
    if(NOT LIBAO)
        message(FATAL_ERROR "libao not found")
    endif()
    file(GLOB AO_RENDERER_SRC plugins/libao/*)
    add_library(AoRenderer SHARED ${AO_RENDERER_SRC})
    target_link_libraries(AoRenderer ${LIBAO})
endif()

# WAV plugin
if(WithPluginWavDecoder)
    file(GLOB WAV_DECODER_SRC plugins/wav/decoder/*)
    add_library(WavDecoder SHARED ${WAV_DECODER_SRC})
endif()
if(WithPluginWavEncoder)
    file(GLOB WAV_ENCODER_SRC plugins/wav/encoder/*)
    add_library(WavEncoder SHARED ${WAV_ENCODER_SRC})
endif()

# MPG123 & Lame plugin
if(WithPluginMPG123)
    find_library(MPG123 mpg123)
    if(NOT MPG123)
        message(FATAL_ERROR "mpg123 not found")
    endif()
    file(GLOB MPG123_DECODER_SRC plugins/mpg123/*)
    add_library(Mpg123Decoder SHARED ${MPG123_DECODER_SRC})
    target_link_libraries(Mpg123Decoder ${MPG123})
endif()
if(WithPluginLAME)
    find_library(LAME mp3lame)
    if(NOT LAME)
        message(FATAL_ERROR "lame not found")
    endif()
    file(GLOB LAME_ENCODER_SRC plugins/lame/*)
    add_library(LameEncoder SHARED ${LAME_ENCODER_SRC})
    target_link_libraries(LameEncoder ${LAME})
endif()

# Ogg plugin
if(WithPluginVorbisDecoder)
    find_library(VORBISFILE vorbisfile)
    if(NOT VORBISFILE)
        message(FATAL_ERROR "libogg not found")
    endif()
    file(GLOB VORBIS_DECODER_SRC plugins/vorbis/decoder/*)
    add_library(VorbisDecoder SHARED ${VORBIS_DECODER_SRC})
    target_link_libraries(VorbisDecoder ${VORBISFILE})
endif()
if(WithPluginVorbisEncoder)
    find_library(VORBIS vorbis)
    find_library(VORBISFILE vorbisfile)
    find_library(VORBISENC vorbisenc)
    find_library(OGG ogg)
    if(NOT VORBIS OR NOT VORBISFILE OR NOT VORBISENC OR NOT OGG)
        message(FATAL_ERROR "libogg not found")
    endif()
    file(GLOB VORBIS_ENCODER_SRC plugins/vorbis/encoder/*)
    add_library(VorbisEncoder SHARED ${VORBIS_ENCODER_SRC})
    target_link_libraries(VorbisEncoder ${VORBIS} ${VORBISFILE} ${VORBISENC} ${OGG})
endif()

# Wma plugin
if(WithPluginWmaDecoder)
    file(GLOB FFWMA_SRC contrib/ffmpeg-strip-wma/*.c)
    file(GLOB WMA_DECODER_SRC plugins/WmaDecoder/*)
    add_library(WmaDecoder SHARED ${FFWMA_SRC} ${WMA_DECODER_SRC})
    set_target_properties(WmaDecoder PROPERTIES 
        COMPILE_FLAGS "-fno-strict-aliasing -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE")
    target_link_libraries(WmaDecoder)
endif()

# Mac plugin
if(WithPluginMacDecoder)
    file(GLOB MAC_DECODER_SRC plugins/MacDecoder/*)
    add_library(MacDecoder SHARED ${MAC_DECODER_SRC})
    target_link_libraries(MacDecoder -lmac)
endif()

# Mac4 plugin(still buggy)
if(WithPluginMac4Decoder)
    if(WithAssembly)
        enable_language(ASM_NASM)
        set(CMAKE_ASM_NASM_COMPILER "yasm")

        if(Bitness EQUAL 64)
            file(GLOB MAC4_SRC_ASM contrib/MACLib/Source/MACLib/Assembly/x86_64_mmx.asm)
            set(YASM_FLAG " -m amd64")
            set(MAC_FLAG " -DENABLE_ASSEMBLY -DARCH_X86_64")
        else()
            file(GLOB MAC4_SRC_ASM contrib/MACLib/Source/MACLib/Assembly/x86_mmx.asm)
            set(YASM_FLAG " -m x86")
            set(MAC_FLAG " -DENABLE_ASSEMBLY -DARCH_X86")
        endif()

        set_source_files_properties(${MAC4_SRC_ASM} COMPILE_FLAGS "-f elf ${YASM_FLAG}") 
    endif()

    file(GLOB MAC4_LIB_SRC  contrib/MACLib/Source/MACLib/*.cpp contrib/MACLib/Source/Shared/*.cpp contrib/MACLib/Source/MACLib/Assembly/*.cpp)
    file(GLOB MAC4_DECODER_SRC plugins/Mac4Decoder/*)

    set_source_files_properties(${MAC4_DECODER_SRC} ${MAC4_LIB_SRC }
        COMPILE_FLAGS "-O2 -fno-strict-aliasing -I${MOUS_SOURCE_DIR}/contrib/MACLib/Source/Shared -DBUILD_CROSS_PLATFORM ${MAC_FLAG}")

    add_library(Mac4Decoder SHARED ${MAC4_DECODER_SRC} ${MAC4_LIB_SRC } ${MAC4_SRC_ASM})
    target_link_libraries(Mac4Decoder)
endif()

# Flac plugin
if(WithPluginFlacDecoder)
    find_library(FLAC FLAC)
    if(NOT FLAC)
        message(FATAL_ERROR "flac not found")
    endif()
    file(GLOB FLAC_DECODER_SRC plugins/flac/*)
    add_library(FlacDecoder SHARED ${FLAC_DECODER_SRC})
    target_link_libraries(FlacDecoder ${FLAC})
endif()

# WavPack plugin
if(WithPluginWvDecoder)
    find_library(WAVPACK wavpack)
    if(NOT WAVPACK)
        message(FATAL_ERROR "wavpack not found")
    endif()
    file(GLOB WV_DECODER_SRC plugins/wavpack/*)
    add_library(WvDecoder SHARED ${WV_DECODER_SRC})
    target_link_libraries(WvDecoder ${WAVPACK})
endif()

# FDK-AAC plugins
if(WithPluginFdkDecoder)
    find_library(FDK_AAC fdk-aac)
    if(NOT FDK_AAC)
        message(FATAL_ERROR "fdk-aac not found")
    endif()
    find_library(MP4V2 mp4v2)
    if(NOT MP4V2)
        message(FATAL_ERROR "mp4v2 not found")
    endif()
    file(GLOB FDK_DECODER_SRC plugins/fdk-aac/decoder/*)
    add_library(FdkDecoder SHARED ${FDK_DECODER_SRC})
    target_link_libraries(FdkDecoder ${FDK_AAC} ${MP4V2})
endif()

# FAAD & FAAC plugin
if(WithPluginFaadDecoder)
    find_library(FAAD faad)
    find_library(MP4FF mp4ff)
    if(NOT FAAD OR NOT MP4FF)
        message(FATAL_ERROR "faad2 not found")
    endif()
    file(GLOB FAAD_DECODER_SRC plugins/faad/*)
    add_library(FaadDecoder SHARED ${FAAD_DECODER_SRC})
    target_link_libraries(FaadDecoder ${FAAD} ${MP4FF})
endif()
if(WithPluginFaacEncoder)
    find_library(FAAC faac)
    if(NOT FAAC)
        message(FATAL_ERROR "faac not found")
    endif()
    find_library(MP4V2 mp4v2)
    if(NOT MP4V2)
        message(FATAL_ERROR "mp4v2 not found")
    endif()
    file(GLOB FAAC_ENCODER_SRC plugins/faac/*)
    add_library(FaacEncoder SHARED ${FAAC_ENCODER_SRC})
    target_link_libraries(FaacEncoder ${FAAC} ${MP4V2})
endif()

# TagLib plugin
if(WithPluginTagLibParser)
    find_library(TAGLIB tag)
    if(NOT TAGLIB)
        message(FATAL_ERROR "taglib not found")
    endif()
    file(GLOB TAGLIB_PARSER_SRC plugins/taglib/*)
    add_library(TagLibParser SHARED ${TAGLIB_PARSER_SRC})
    if(IS_FREEBSD OR IS_APPLE)
        target_link_libraries(TagLibParser ${TAGLIB} -liconv)
    elseif(IS_LINUX)
        target_link_libraries(TagLibParser ${TAGLIB})
    endif()
endif()

# CuePack plugin
if(WithPluginCuePack)
    find_library(LIBCUE cue)
    if(NOT LIBCUE)
        message(FATAL_ERROR "libcue not found")
    endif()
    file(GLOB CUE_PACK_SRC plugins/libcue/*)
    add_library(CuePack SHARED ${CUE_PACK_SRC})
    target_link_libraries(CuePack ${LIBCUE})
endif()

# PlayerStatistics plugin
#if(WithPluginPlayerStatistics)
#    file(GLOB PLAYERSTATISTICS_SRC plugins/PlayerStatistics/*)
#    add_library(PlayerStatistics SHARED ${PLAYERSTATISTICS_SRC})
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
    CODE "file(GLOB PLUGIN_FILES \"${PROJECT_BINARY_DIR}/plugins/*\")"
    CODE "file(INSTALL \${PLUGIN_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/mous)"
)

#file(GLOB RESOURCE_FILES "${MOUS_SOURCE_DIR}/resource/*")
#install(DIRECTORY ${RESOURCE_FILES} DESTINATION share/mous)

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
