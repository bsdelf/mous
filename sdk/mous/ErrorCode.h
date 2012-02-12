#ifndef MOUS_ERRORCODE_H
#define MOUS_ERRORCODE_H

namespace mous {

enum ErrorCode {
    MousOk = 0,

    MousFileNotFound,

    MousMgrFailedToOpen,
    MousMgrBadFormat,

    MousDecoderFailedToInit,
    MousDecoderFailedToOpen,
    MousDecoderOutOfRange,

    MousRendererFailedToOpen,
    MousRendererFailedToSetup,
    MousRendererFailedToWrite,
    MousRendererBadChannels,
    MousRendererBadSampleRate,
    MousRendererBadBitsPerSample,
    MousRendererIsNotSupported,

    MousPlayerNoDecoder,
    MousPlayerNoRenderer
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

#endif
