#include "Decoder.h"
#include <util/PluginHelper.h>

static const PluginInfo plugin_info {
    "fdk-aac",
    "FDK Decoder",
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
