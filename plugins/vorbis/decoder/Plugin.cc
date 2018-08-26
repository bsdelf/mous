#include <util/PluginHelper.h>
#include "Decoder.h"

static const PluginInfo plugin_info {
    "vorbis",
    "Decoder for Ogg Vorbis stream.",
    2
};

extern "C" {
    PluginType MousGetPluginType() {
        return PluginType::Decoder;
    }

    const PluginInfo* MousGetPluginInfo() {
        return &plugin_info;
    }

    void* MousCreateObject() {
        return nullptr;
    }

    void MousFreeObject() {
    }

    const DecoderInterface* MousGetDecoderInterface() {
        return &decoder_interface;
    }
}
