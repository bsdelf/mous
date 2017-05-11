#pragma once

#include <inttypes.h>

namespace mous {

enum class ErrorCode : uint8_t
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

    RendererFailedToOpen,
    RendererFailedToSetup,
    RendererFailedToWrite,
    RendererBadChannels,
    RendererBadSampleRate,
    RendererBadBitsPerSample,
    RendererIsNotSupported,

    PlayerNoDecoder,
    PlayerNoRenderer,

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

    "Renderer is not supported!",

    "File not found!",
    "Out of range!"
};
*/

}
