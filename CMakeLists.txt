cmake_minimum_required(VERSION 3.4)

project(MOUS)

#=== Compiler Settings ====#
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

#==== Library path ====#
include_directories(SYSTEM /usr/local/include)
link_directories(/usr/local/lib)
include_directories(BEFORE sdk)
include_directories(BEFORE deps)

#==== threading ====#
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#==== iconv ====#
find_library(ICONV NAMES iconv libiconv)
if(ICONV)
    message(STATUS "iconv: ${ICONV}")
else()
    set(ICONV "")
    message(STATUS "iconv: libc")
endif()

SET(CMAKE_INSTALL_RPATH "\${CMAKE_INSTALL_PREFIX}/lib:\${CMAKE_INSTALL_PREFIX}/bin")

#==== OS Check ====#
if(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
    set(FREEBSD true)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX true)
endif()

if(NOT APPLE)
    add_definitions(-DUSE_POSIX_SEMAPHORE)
endif()

#==== amd64/x86 ====#
set(BITNESS 32)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(BITNESS 64)
endif()

set(CMAKE_INSTALL_PREFIX "/usr/local" CACHE PATH "Install path prefix")
add_definitions(-DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")

#==== Frontend ====#
option(WITH_QT4_FRONTEND "Build Qt frontend" off)
option(WITH_NCURSES_FRONTEND "Build ncurses frontend" off)

#==== Optional ====#
option(WITH_ASSEMBLY "Enable Assembly Optimize" on)
option(WITH_SDK "Install SDK" off)

option(WITH_OSS_RENDERER "OSS Output" off)
option(WITH_ALSA_RENDERER "ALSA output" off)
option(WITH_CORE_AUDIO_RENDERER "Core Audio Output" off)
option(WITH_AO_RENDERER "AO output" off)

option(WITH_WAV_DECODER "WAV Decoder" on)
option(WITH_WAV_ENCODER "WAV Encoder" on)
option(WITH_WMA_DECODER "WMA Decoder" off)
option(WITH_MAC_DECODER "MAC Decoder" off)
option(WITH_MAC4_DECODER "MAC4.11 Decoder" off)
option(WITH_MPG123_DECODER "MPG123 Decoder" off)
option(WITH_VORBIS_DECODER "Ogg Decoder" off)
option(WITH_VORBIS_ENCODER "Ogg Encoder" off)
option(WITH_LAME_ENCODER "LAME Encoder" off)
option(WITH_FLAC_DECODER "FLAC Decoder" off)
option(WITH_FLAC_ENCODER "FLAC Encoder" off)
option(WITH_FDK_DECODER "Fdk Decoder" off)
option(WITH_FAAD_DECODER "FAAD Decoder" off)
option(WITH_FAAC_ENCODER "FAAC Encoder" off)
option(WITH_WV_DECODER "WavPack Decoder" off)

option(WITH_TAGLIB_PLUGIN "TagLib Plugin" on)
option(WITH_CUE_PLUGIN "CUE Sheet Plugin" on)

#option(WithPluginPlayerStatistics "Player activity statistics" on)

#==== Output Dir ====#
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/plugins)

# libCharsetConv
#file(GLOB_RECURSE LIB_CHARSET_CONV charset/*.cpp charset/uchardect/src/*)
#add_library(CharsetConv SHARED ${LIB_CHARSET_CONV})
#if(NOT LINUX)
#    target_link_libraries(CharsetConv -liconv)
#endif()

#==== Targets ====#

# libMousCore
file(GLOB_RECURSE LIB_MOUS_CORE core/*)
add_library(MousCore SHARED ${LIB_MOUS_CORE})
set_target_properties(MousCore PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
if(FREEBSD OR APPLE)
    target_link_libraries(MousCore Threads::Threads)
elseif(LINUX)
    target_link_libraries(MousCore Threads::Threads -ldl)
endif()

# mous-cli
file(GLOB_RECURSE EXE_MOUS_CLI frontend/cli/*)
add_executable(mous-cli ${EXE_MOUS_CLI})
add_dependencies(mous-cli MousCore)
if(FREEBSD OR APPLE)
    target_link_libraries(mous-cli MousCore)
elseif(LINUX)
    target_link_libraries(mous-cli MousCore -ldl)
endif()

# mous-ncurses
if(WITH_NCURSES_FRONTEND)
    find_library(NCURSES NAMES ncursesw ncurses)
    if(NOT NCURSES)
        message(FATAL_ERROR "ncurses/ncursesw not found")
    endif()
    file(GLOB EXE_MOUS_NCURSES frontend/ncurses/*)
    add_executable(mous-ncurses ${EXE_MOUS_NCURSES})
    add_dependencies(mous-ncurses MousCore)
    set(EXE_MOUS_NCURSES_LIBS MousCore Threads::Threads ${NCURSES} ${ICONV})
    if(LINUX)
        target_link_libraries(mous-ncurses ${EXE_MOUS_NCURSES_LIBS} -ldl -lrt)
    else()
        target_link_libraries(mous-ncurses ${EXE_MOUS_NCURSES_LIBS})
    endif()
endif()

# mous-qt
if(WITH_QT4_FRONTEND)
    find_package(Qt4 REQUIRED QtCore QtGui)

    include(${QT_USE_FILE})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    set(EXE_MOUS_QT_MOC_HEADERS)
    file(GLOB EXE_MOUS_QT_HEADERS frontend/qt4/*.h frontend/qt4/*.hpp)
    foreach(header ${EXE_MOUS_QT_HEADERS})
        file(STRINGS "${header}" lines REGEX "Q_OBJECT")
        if(lines)
            list(APPEND EXE_MOUS_QT_MOC_HEADERS "${header}")
        endif()
    endforeach()
    QT4_WRAP_CPP(EXE_MOUS_QT_WCPP ${EXE_MOUS_QT_MOC_HEADERS})

    file(GLOB EXE_MOUS_QT_UI frontend/qt4/*.ui)
    QT4_WRAP_UI(EXE_MOUS_QT_WUI ${EXE_MOUS_QT_UI})

    file(GLOB EXE_MOUS_QT_QRC frontend/qt4/*.qrc)
    QT4_ADD_RESOURCES(EXE_MOUS_QT_RES ${EXE_MOUS_QT_QRC})

    file(GLOB EXE_MOUS_QT_TS frontend/qt4/*.ts)
    QT4_ADD_TRANSLATION(EXE_MOUS_QT_QM ${EXE_MOUS_QT_TS})

    file(GLOB EXE_MOUS_QT_SOURCES frontend/qt4/*.cpp)
    add_executable(mous-qt ${EXE_MOUS_QT_SOURCES}
        ${EXE_MOUS_QT_WCPP} ${EXE_MOUS_QT_WUI} ${EXE_MOUS_QT_RES} ${EXE_MOUS_QT_QM})
    add_dependencies(mous-qt MousCore)
    target_link_libraries(mous-qt MousCore ${QT_LIBRARIES} ${ICONV})
endif()

# OSS plugin
if(WITH_OSS_RENDERER)
    file(GLOB OSS_RENDERER_SRC plugins/oss/*)
    add_library(OssRenderer SHARED ${OSS_RENDERER_SRC})
    target_link_libraries(OssRenderer)
endif()

# ALSA plugin
if(WITH_ALSA_RENDERER)
    file(GLOB ALSA_RENDERER_SRC plugins/alsa/*)
    add_library(AlsaRenderer SHARED ${ALSA_RENDERER_SRC})
    target_link_libraries(AlsaRenderer -lasound)
endif()

# Core Audio plugin
if(WITH_CORE_AUDIO_RENDERER)
    find_library(AUDIO_UNIT AudioUnit)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    find_library(CORE_AUDIO CoreAudio)
    file(GLOB_RECURSE CORE_AUDIO_RENDERER_SRC plugins/coreaudio/*)
    add_library(CoreAudioRenderer SHARED ${CORE_AUDIO_RENDERER_SRC})
    target_link_libraries(CoreAudioRenderer ${AUDIO_UNIT} ${AUDIO_TOOLBOX} ${CORE_AUDIO})
endif()

# AO plugin
if(WITH_AO_RENDERER)
    find_library(LIBAO ao)
    if(NOT LIBAO)
        message(FATAL_ERROR "libao not found")
    endif()
    file(GLOB AO_RENDERER_SRC plugins/libao/*)
    add_library(AoRenderer SHARED ${AO_RENDERER_SRC})
    target_link_libraries(AoRenderer ${LIBAO})
endif()

# WAV plugin
if(WITH_WAV_DECODER)
    file(GLOB WAV_DECODER_SRC plugins/wav/decoder/*)
    add_library(WavDecoder SHARED ${WAV_DECODER_SRC})
endif()
if(WITH_WAV_ENCODER)
    file(GLOB WAV_ENCODER_SRC plugins/wav/encoder/*)
    add_library(WavEncoder SHARED ${WAV_ENCODER_SRC})
endif()

# MPG123 & Lame plugin
if(WITH_MPG123_DECODER)
    find_library(MPG123 mpg123)
    if(NOT MPG123)
        message(FATAL_ERROR "mpg123 not found")
    endif()
    file(GLOB MPG123_DECODER_SRC plugins/mpg123/*)
    add_library(Mpg123Decoder SHARED ${MPG123_DECODER_SRC})
    target_link_libraries(Mpg123Decoder ${MPG123})
endif()
if(WITH_LAME_ENCODER)
    find_library(LAME mp3lame)
    if(NOT LAME)
        message(FATAL_ERROR "lame not found")
    endif()
    file(GLOB LAME_ENCODER_SRC plugins/lame/*)
    add_library(LameEncoder SHARED ${LAME_ENCODER_SRC})
    target_link_libraries(LameEncoder ${LAME})
endif()

# Ogg plugin
if(WITH_VORBIS_DECODER)
    find_library(VORBISFILE vorbisfile)
    if(NOT VORBISFILE)
        message(FATAL_ERROR "libogg not found")
    endif()
    file(GLOB VORBIS_DECODER_SRC plugins/vorbis/decoder/*)
    add_library(VorbisDecoder SHARED ${VORBIS_DECODER_SRC})
    target_link_libraries(VorbisDecoder ${VORBISFILE})
endif()
if(WITH_VORBIS_ENCODER)
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
if(WITH_WMA_DECODER)
    file(GLOB FFWMA_SRC deps/ffmpeg-strip-wma/*.c)
    file(GLOB WMA_DECODER_SRC plugins/WmaDecoder/*)
    add_library(WmaDecoder SHARED ${FFWMA_SRC} ${WMA_DECODER_SRC})
    set_target_properties(WmaDecoder PROPERTIES 
        COMPILE_FLAGS "-fno-strict-aliasing -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE")
    target_link_libraries(WmaDecoder)
endif()

# Mac plugin
if(WITH_MAC_DECODER)
    file(GLOB MAC_DECODER_SRC plugins/MacDecoder/*)
    add_library(MacDecoder SHARED ${MAC_DECODER_SRC})
    target_link_libraries(MacDecoder -lmac)
endif()

# Mac4 plugin(still buggy)
if(WITH_MAC4_DECODER)
    if(WITH_ASSEMBLY)
        enable_language(ASM_NASM)
        set(CMAKE_ASM_NASM_COMPILER "yasm")

        if(BITNESS EQUAL 64)
            file(GLOB MAC4_SRC_ASM deps/MACLib/Source/MACLib/Assembly/x86_64_mmx.asm)
            set(YASM_FLAG " -m amd64")
            set(MAC_FLAG " -DENABLE_ASSEMBLY -DARCH_X86_64")
        else()
            file(GLOB MAC4_SRC_ASM deps/MACLib/Source/MACLib/Assembly/x86_mmx.asm)
            set(YASM_FLAG " -m x86")
            set(MAC_FLAG " -DENABLE_ASSEMBLY -DARCH_X86")
        endif()

        set_source_files_properties(${MAC4_SRC_ASM} COMPILE_FLAGS "-f elf ${YASM_FLAG}") 
    endif()

    file(GLOB MAC4_LIB_SRC  deps/MACLib/Source/MACLib/*.cpp deps/MACLib/Source/Shared/*.cpp deps/MACLib/Source/MACLib/Assembly/*.cpp)
    file(GLOB MAC4_DECODER_SRC plugins/Mac4Decoder/*)

    set_source_files_properties(${MAC4_DECODER_SRC} ${MAC4_LIB_SRC}
        COMPILE_FLAGS "-O2 -fno-strict-aliasing -I${MOUS_SOURCE_DIR}/deps/MACLib/Source/Shared -DBUILD_CROSS_PLATFORM ${MAC_FLAG}")

    add_library(Mac4Decoder SHARED ${MAC4_DECODER_SRC} ${MAC4_LIB_SRC} ${MAC4_SRC_ASM})
    target_link_libraries(Mac4Decoder)
endif()

# Flac plugin
if(WITH_FLAC_DECODER)
    find_library(FLAC FLAC)
    if(NOT FLAC)
        message(FATAL_ERROR "flac not found")
    endif()
    file(GLOB FLAC_DECODER_SRC plugins/flac/*)
    add_library(FlacDecoder SHARED ${FLAC_DECODER_SRC})
    target_link_libraries(FlacDecoder ${FLAC})
endif()

# WavPack plugin
if(WITH_WV_DECODER)
    find_library(WAVPACK wavpack)
    if(NOT WAVPACK)
        message(FATAL_ERROR "wavpack not found")
    endif()
    file(GLOB WV_DECODER_SRC plugins/wavpack/*)
    add_library(WvDecoder SHARED ${WV_DECODER_SRC})
    target_link_libraries(WvDecoder ${WAVPACK})
endif()

# FDK-AAC plugins
if(WITH_FDK_DECODER)
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
if(WITH_FAAD_DECODER)
    find_library(FAAD faad)
    find_library(MP4FF mp4ff)
    if(NOT FAAD OR NOT MP4FF)
        message(FATAL_ERROR "faad2 not found")
    endif()
    file(GLOB FAAD_DECODER_SRC plugins/faad/*)
    add_library(FaadDecoder SHARED ${FAAD_DECODER_SRC})
    target_link_libraries(FaadDecoder ${FAAD} ${MP4FF})
endif()
if(WITH_FAAC_ENCODER)
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
if(WITH_TAGLIB_PLUGIN)
    find_library(TAGLIB tag)
    if(NOT TAGLIB)
        message(FATAL_ERROR "taglib not found")
    endif()
    file(GLOB TAGLIB_PARSER_SRC plugins/taglib/*)
    add_library(TagLibParser SHARED ${TAGLIB_PARSER_SRC})
    target_link_libraries(TagLibParser ${TAGLIB} ${ICONV})
endif()

# CueParser plugin
if(WITH_CUE_PLUGIN)
    find_library(LIBCUE cue)
    if(NOT LIBCUE)
        message(FATAL_ERROR "libcue not found")
    endif()
    file(GLOB CUE_PACK_SRC plugins/libcue/*)
    add_library(CueParser SHARED ${CUE_PACK_SRC})
    target_link_libraries(CueParser ${LIBCUE})
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

if(WITH_NCURSES_FRONTEND)
    install(TARGETS mous-ncurses
        RUNTIME DESTINATION bin)
endif()

if(WITH_QT4_FRONTEND)
    install(TARGETS mous-qt
        RUNTIME DESTINATION bin)
    install(
        CODE "file(GLOB TRANSLATION_FILES \"${PROJECT_BINARY_DIR}/*.qm\")"
        CODE "file(INSTALL \${TRANSLATION_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mous/qt)"
    )
endif()

if(WITH_SDK)
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
message(STATUS "CMake Version:  " ${CMAKE_VERSION})
message(STATUS "System Name:    " ${CMAKE_SYSTEM_NAME})
message(STATUS "System Version: " ${CMAKE_SYSTEM_VERSION})
message(STATUS "C Compiler:     " ${CMAKE_C_COMPILER})
message(STATUS "C++ Compiler:   " ${CMAKE_CXX_COMPILER})
message(STATUS "Processor:      " ${CMAKE_SYSTEM_PROCESSOR})
message(STATUS "Build Type:     " ${CMAKE_BUILD_TYPE})
message(STATUS "Project Root:   " ${MOUS_SOURCE_DIR})
message(STATUS "Build Root:     " ${PROJECT_BINARY_DIR})
message(STATUS "For Arch:       " ${BITNESS})
