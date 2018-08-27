#include <util/PluginHelper.h>
using namespace mous;

static const PluginInfo plugin_info {
    "coreaudio",
    "Core Audio Output",
    2
};

extern "C" {
    PluginType MousGetPluginType() {
        return PluginType::Output;
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
