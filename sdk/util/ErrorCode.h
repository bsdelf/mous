#pragma once

#include <inttypes.h>

namespace mous {

enum class ErrorCode : uint32_t
{
    Ok = 0,

    FileNotFound,

    PluginFailedToOpen,
    PluginBadFormat,

    DecoderFailedToOpen,
    DecoderFailedToInit,
    DecoderFailedToRead,
    DecoderOutOfRange,

    EncoderFailedToOpen,
    EncoderFailedToInit,
    EncoderFailedToEncode,
    EncoderFailedToFlush,

    OutputFailedToOpen,
    OutputFailedToSetup,
    OutputFailedToWrite,
    OutputBadChannels,
    OutputBadSampleRate,
    OutputBadBitsPerSample,
    OutputIsNotSupported,

    PlayerNoDecoder,
    PlayerNoOutput,

    TagPraserFailedToWrite,

    MediaLoaderFailedToLoad,

    PlaylistEmpty,
    PlaylistHitBegin,
    PlaylistHitEnd
};

/*
const char* const ErrorMsg[] = {
    "OK.",

    "Decoder failed to initialize!",
    "Decoder failed to open file!"

    "Bad format!",
    "Format is not supported!",

    "Output is not supported!",

    "File not found!",
    "Out of range!"
};
*/

}
