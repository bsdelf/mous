#include <util/PluginHelper.h>
using namespace mous;

static const PluginInfo plugin_info {
    "wav",
    "WAV Codec",
    2
};

extern "C" {
    PluginType MousGetPluginType() {
        return PluginType::Decoder | PluginType::Encoder;
    }

    const PluginInfo* MousGetPluginInfo() {
        return &plugin_info;
    }

    void* MousCreateObject() {
        return nullptr;
    }

    void MousFreeObject() {
    }
}
