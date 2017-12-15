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

#==== Detect ====#
find_library(LIB_AO ao)
find_library(LIB_AUDIO_TOOLBOX AudioToolbox)
find_library(LIB_AUDIO_UNIT AudioUnit)
find_library(LIB_CORE_AUDIO CoreAudio)
find_library(LIB_CUE cue)
find_library(LIB_FAAC faac)
find_library(LIB_FAAD faad)
find_library(LIB_FDK_AAC fdk-aac)
find_library(LIB_FLAC FLAC)
find_library(LIB_ICONV NAMES iconv libiconv)
find_library(LIB_LAME mp3lame)
find_library(LIB_MP4FF mp4ff)
find_library(LIB_MP4V2 mp4v2)
find_library(LIB_MPG123 mpg123)
find_library(LIB_NCURSES NAMES ncursesw ncurses)
find_library(LIB_OGG ogg)
find_library(LIB_TAGLIB tag)
find_library(LIB_VORBIS vorbis)
find_library(LIB_VORBISENC vorbisenc)
find_library(LIB_VORBISFILE vorbisfile)
find_library(LIB_WAVPACK wavpack)
if(APPLE)
    find_program(PROG_BREW brew)
    if(PROG_BREW)
        execute_process(
            COMMAND ${PROG_BREW} --prefix qt5
            ERROR_QUIET
            TIMEOUT 5
            OUTPUT_STRIP_TRAILING_WHITESPACE
            OUTPUT_VARIABLE PATH_QT5_PREFIX)
        if(EXISTS "${PATH_QT5_PREFIX}/lib/cmake/Qt5")
            set(Qt5_DIR "${PATH_QT5_PREFIX}/lib/cmake/Qt5")
        endif()
    endif()
    unset(PROG_BREW CACHE)
endif()
find_package(Qt5 QUIET COMPONENTS Widgets)
if(LIB_ICONV)
    message(STATUS "iconv: ${LIB_ICONV}")
else()
    set(LIB_ICONV "")
    message(STATUS "iconv: libc")
endif()

#==== APP ====#
option(WITH_APP_CLI "Build CLI application" ON)
if(Qt5Widgets_DIR)
    set(TRY_APP_QT5 ON)
endif()
option(WITH_APP_QT5 "Build Qt5 application" ${TRY_APP_QT5})
if(LIB_NCURSES)
    set(TRY_APP_NCURSES ON)
endif()
option(WITH_APP_NCURSES "Build ncurses application" ${TRY_APP_NCURSES})

#==== SDK ====#
option(WITH_SDK "Install SDK" OFF)

#==== Utils ====#
if(LIB_CUE)
    set(TRY_PLUGIN_LIBCUE ON)
endif()
option(WITH_PLUGIN_LIBCUE "CUE Sheet plugin" ${TRY_PLUGIN_LIBCUE})
if(LIB_TAGLIB)
    set(TRY_PLUGIN_TAGLIB ON)
endif()
option(WITH_PLUGIN_TAGLIB "TagLib plugin" ${TRY_PLUGIN_TAGLIB})

#==== Outputs ====#
option(WITH_PLUGIN_ALSA "ALSA output" ${TRY_PLUGIN_ALSA})
option(WITH_PLUGIN_LIBAO "Libao output" OFF)
if(LIB_AUDIO_UNIT AND LIB_AUDIO_TOOLBOX AND LIB_CORE_AUDIO)
    set(TRY_PLUGIN_COREAUDIO ON)
endif()
option(WITH_PLUGIN_COREAUDIO "Core Audio output" ${TRY_PLUGIN_COREAUDIO})
if(FREEBSD)
    set(TRY_PLUGIN_OSS ON)
endif()
option(WITH_PLUGIN_OSS "OSS output" ${TRY_PLUGIN_OSS})

#==== Codecs ====#
if(LIB_FAAC AND LIB_MP4V2 AND FALSE)
    set(TRY_PLUGIN_FAAC ON)
endif()
option(WITH_PLUGIN_FAAC "FAAC encoder" ${TRY_PLUGIN_FAAC})
if(LIB_FAAD AND LIB_MP4FF AND FALSE)
    set(TRY_PLUGIN_FAAD ON)
endif()
option(WITH_PLUGIN_FAAD "FAAD2 decoder" ${TRY_PLUGIN_FAAD})
if(LIB_FDK_AAC AND LIB_MP4V2)
    set(TRY_PLUGIN_FDK_AAC ON)
endif()
option(WITH_PLUGIN_FDK_AAC "FDK AAC decoder" ${TRY_PLUGIN_FDK_AAC})
if(LIB_FLAC)
    set(TRY_PLUGIN_FLAC ON)
endif()
option(WITH_PLUGIN_FLAC "FLAC decoder" ${TRY_PLUGIN_FLAC})
if(LIB_LAME)
    set(TRY_PLUGIN_LAME ON)
endif()
option(WITH_PLUGIN_LAME "LAME encoder" ${TRY_PLUGIN_LAME})
option(WITH_PLUGIN_MAC "MAC decoder" OFF)
if(LIB_MPG123)
    set(TRY_PLUGIN_MPG123 ON)
endif()
option(WITH_PLUGIN_MPG123 "mpg123 decoder" ${TRY_PLUGIN_MPG123})
if(LIB_OGG AND LIB_VORBIS AND LIB_VORBISFILE AND LIB_VORBISENC)
    set(TRY_PLUGIN_OGG_VORBIS)
endif()
option(WITH_PLUGIN_OGG_VORBIS "Ogg Vorbis codec" ${TRY_PLUGIN_OGG_VORBIS})
option(WITH_PLUGIN_WAV "WAV codec" ON)
if(LIB_WAVPACK)
    set(TRY_PLUGIN_WAVPACK ON)
endif()
option(WITH_PLUGIN_WAVPACK "WavPack decoder" ${TRY_PLUGIN_WAVPACK})
option(WITH_PLUGIN_WMA_DECODER "WMA decoder" OFF)

#option(WithPluginPlayerStatistics "Player activity statistics" ON)

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
if(LINUX)
    set(LIB_MOUS_CORE_EXTRA -ld)
endif()
target_link_libraries(MousCore Threads::Threads ${LIB_MOUS_CORE_EXTRA})

if(WITH_APP_CLI)
    file(GLOB_RECURSE APP_CLI_SRC apps/cli/*)
    add_executable(mous-cli ${APP_CLI_SRC})
    add_dependencies(mous-cli MousCore)
    if(LINUX)
        set(LIB_MOUS_CLI_EXTRA -ldl -lrt)
    endif()
    target_link_libraries(mous-cli MousCore ${LIB_MOUS_CLI_EXTRA})
endif()

if(WITH_APP_NCURSES)
    file(GLOB APP_NCURSES_SRC apps/ncurses/*)
    add_executable(mous-ncurses ${APP_NCURSES_SRC})
    add_dependencies(mous-ncurses MousCore)
    if(LINUX)
        set(LIB_MOUS_NCURSES_EXTRA -ldl -lrt)
    endif()
    target_link_libraries(mous-ncurses MousCore Threads::Threads ${LIB_NCURSES} ${LIB_ICONV} ${LIB_MOUS_NCURSES_EXTRA})
endif()

if(WITH_APP_QT5)
    #==== src ====#
    file(GLOB APP_QT5_SRC apps/qt5/*.cpp)
    #==== moc ====#
    file(GLOB APP_QT5_HEADERS apps/qt5/*.h apps/qt5/*.hpp)
    set(APP_QT5_MOC_FILES)
    foreach(header ${APP_QT5_HEADERS})
        file(STRINGS "${header}" lines REGEX "Q_OBJECT")
        if(lines)
            list(APPEND APP_QT5_MOC_FILES "${header}")
        endif()
    endforeach()
    qt5_wrap_cpp(APP_QT5_MOC ${APP_QT5_MOC_FILES})
    #==== ui ====#
    file(GLOB APP_QT5_UI_FILES apps/qt5/*.ui)
    qt5_wrap_ui(APP_QT5_UI ${APP_QT5_UI_FILES})
    #==== qrc ====#
    file(GLOB APP_QT5_QRC_FILES apps/qt5/*.qrc)
    qt5_add_resources(APP_QT5_QRC ${APP_QT5_QRC_FILES})
    #==== ts ====#
    #file(GLOB APP_QT5_TS_FILES apps/qt5/*.ts)
    #qt5_add_translation(APP_QT5_TS ${APP_QT5_TS_FILES})
    #==== build ====#
    add_executable(mous-qt ${APP_QT5_SRC} ${APP_QT5_MOC} ${APP_QT5_UI} ${APP_QT5_QRC} ${APP_QT5_TS})
    add_dependencies(mous-qt MousCore)
    target_include_directories(mous-qt PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
    target_link_libraries(mous-qt MousCore Qt5::Widgets ${LIB_ICONV})
endif()

if(WITH_PLUGIN_OSS)
    file(GLOB OSS_RENDERER_SRC plugins/oss/*)
    add_library(OssRenderer SHARED ${OSS_RENDERER_SRC})
    target_link_libraries(OssRenderer)
endif()

if(WITH_PLUGIN_ALSA)
    file(GLOB ALSA_RENDERER_SRC plugins/alsa/*)
    add_library(AlsaRenderer SHARED ${ALSA_RENDERER_SRC})
    target_link_libraries(AlsaRenderer -lasound)
endif()

if(WITH_PLUGIN_COREAUDIO)
    file(GLOB_RECURSE CORE_AUDIO_RENDERER_SRC plugins/coreaudio/*)
    add_library(CoreAudioRenderer SHARED ${CORE_AUDIO_RENDERER_SRC})
    target_link_libraries(CoreAudioRenderer ${LIB_AUDIO_UNIT} ${LIB_AUDIO_TOOLBOX} ${LIB_CORE_AUDIO})
endif()

if(WITH_PLUGIN_LIBAO)
    file(GLOB AO_RENDERER_SRC plugins/libao/*)
    add_library(AoRenderer SHARED ${AO_RENDERER_SRC})
    target_link_libraries(AoRenderer ${LIB_AO})
endif()

if(WITH_PLUGIN_WAV)
    file(GLOB WAV_DECODER_SRC plugins/wav/decoder/*)
    add_library(WavDecoder SHARED ${WAV_DECODER_SRC})
    file(GLOB WAV_ENCODER_SRC plugins/wav/encoder/*)
    add_library(WavEncoder SHARED ${WAV_ENCODER_SRC})
endif()

if(WITH_PLUGIN_MPG123)
    file(GLOB MPG123_DECODER_SRC plugins/mpg123/*)
    add_library(Mpg123Decoder SHARED ${MPG123_DECODER_SRC})
    target_link_libraries(Mpg123Decoder ${LIB_MPG123})
endif()

if(WITH_PLUGIN_LAME)
    file(GLOB LAME_ENCODER_SRC plugins/lame/*)
    add_library(LameEncoder SHARED ${LAME_ENCODER_SRC})
    target_link_libraries(LameEncoder ${LIB_LAME})
endif()

if(WITH_PLUGIN_OGG_VORBIS)
    file(GLOB VORBIS_DECODER_SRC plugins/vorbis/decoder/*)
    add_library(VorbisDecoder SHARED ${VORBIS_DECODER_SRC})
    target_link_libraries(VorbisDecoder ${LIB_VORBISFILE})
    file(GLOB VORBIS_ENCODER_SRC plugins/vorbis/encoder/*)
    add_library(VorbisEncoder SHARED ${VORBIS_ENCODER_SRC})
    target_link_libraries(VorbisEncoder ${LIB_OGG} ${LIB_VORBIS} ${LIB_VORBISFILE} ${LIB_VORBISENC})
endif()

if(WITH_PLUGIN_WMA_DECODER)
    file(GLOB FFWMA_SRC deps/ffmpeg-strip-wma/*.c)
    file(GLOB WMA_DECODER_SRC plugins/WmaDecoder/*)
    add_library(WmaDecoder SHARED ${FFWMA_SRC} ${WMA_DECODER_SRC})
    set_target_properties(WmaDecoder PROPERTIES 
        COMPILE_FLAGS "-fno-strict-aliasing -DHAVE_AV_CONFIG_H -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_GNU_SOURCE")
    target_link_libraries(WmaDecoder)
endif()

if(WITH_PLUGIN_MAC)
    file(GLOB MAC_SDK_SRC
        deps/MAC_SDK/Source/MACLib/*.cpp
        deps/MAC_SDK/Source/MACLib/Old/*.cpp
        deps/MAC_SDK/Source/Shared/GlobalFunctions.cpp
        deps/MAC_SDK/Source/Shared/CircleBuffer.cpp
        deps/MAC_SDK/Source/Shared/StdLibFileIO.cpp
        deps/MAC_SDK/Source/Shared/CharacterHelper.cpp
    )
    file(GLOB MAC_DECODER_SRC plugins/mac/*)
    add_library(MacDecoder SHARED ${MAC_DECODER_SRC} ${MAC_SDK_SRC})
    set_target_properties(MacDecoder PROPERTIES 
        COMPILE_FLAGS "-DPLATFORM_LINUX -I${MOUS_SOURCE_DIR}/deps/MAC_SDK/Source/Shared")
    target_link_libraries(MacDecoder)
endif()

if(WITH_PLUGIN_FLAC)
    file(GLOB FLAC_DECODER_SRC plugins/flac/*)
    add_library(FlacDecoder SHARED ${FLAC_DECODER_SRC})
    target_link_libraries(FlacDecoder ${LIB_FLAC})
endif()

if(WITH_PLUGIN_WAVPACK)
    file(GLOB WV_DECODER_SRC plugins/wavpack/*)
    add_library(WvDecoder SHARED ${WV_DECODER_SRC})
    target_link_libraries(WvDecoder ${LIB_WAVPACK})
endif()

if(WITH_PLUGIN_FDK_AAC)
    file(GLOB FDK_DECODER_SRC plugins/fdk-aac/decoder/*)
    add_library(FdkDecoder SHARED ${FDK_DECODER_SRC})
    target_link_libraries(FdkDecoder ${LIB_FDK_AAC} ${LIB_MP4V2})
endif()

if(WITH_PLUGIN_FAAD)
    file(GLOB FAAD_DECODER_SRC plugins/faad/*)
    add_library(FaadDecoder SHARED ${FAAD_DECODER_SRC})
    target_link_libraries(FaadDecoder ${LIB_FAAD} ${LIB_MP4FF})
endif()

if(WITH_PLUGIN_FAAC)
    file(GLOB FAAC_ENCODER_SRC plugins/faac/*)
    add_library(FaacEncoder SHARED ${FAAC_ENCODER_SRC})
    target_link_libraries(FaacEncoder ${LIB_FAAC} ${LIB_MP4V2})
endif()

if(WITH_PLUGIN_TAGLIB)
    file(GLOB TAGLIB_PARSER_SRC plugins/taglib/*)
    add_library(TagLibParser SHARED ${TAGLIB_PARSER_SRC})
    target_link_libraries(TagLibParser ${LIB_TAGLIB} ${LIB_ICONV})
endif()

if(WITH_PLUGIN_LIBCUE)
    file(GLOB CUE_PACK_SRC plugins/libcue/*)
    add_library(CueParser SHARED ${CUE_PACK_SRC})
    target_link_libraries(CueParser ${LIB_CUE})
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

if(WITH_APP_NCURSES)
    install(TARGETS mous-ncurses
        RUNTIME DESTINATION bin)
endif()

if(WITH_APP_QT5)
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
