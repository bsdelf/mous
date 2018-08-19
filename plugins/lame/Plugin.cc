#include <util/PluginHelper.h>
#include "Encoder.h"

static const PluginInfo plugin_info = {
    "lame",
    "Lame Encoder",
    1
};

extern "C" {
    PluginType MousGetPluginType() {
        return PluginType::Encoder;
    }

    const PluginInfo* MousGetPluginInfo() {
        return &plugin_info;
    }

    void* MousCreateObject() {
        return nullptr;
    }

    void MousFreeObject() {
    }

    const EncoderInterface* MousGetEncoderInterface() {
        return &encoder_interface;
    }
}