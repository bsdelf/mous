#include <util/PluginHelper.h>
using namespace mous;

static const PluginInfo plugin_info {
    "vorbis",
    "Ogg Vorbis Codec",
    2
};

extern "C" {
    PluginType MousGetPluginType() {
        return PluginType::Decoder/* | PluginType::Encoder*/;
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
