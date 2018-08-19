#include "Decoder.h"
#include <util/PluginDef.h>

static const PluginInfo plugin_info {
    "mac",
    "MAC Decoder",
    1
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
