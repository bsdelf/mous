#include <util/PluginHelper.h>
using namespace mous;

static const PluginInfo plugin_info = {
    "lame",
    "Lame Encoder",
    2
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
}
