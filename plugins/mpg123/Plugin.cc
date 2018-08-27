#include <util/PluginHelper.h>
using namespace mous;

static const PluginInfo plugin_info {
    "mpg123",
    "MPG123 Decoder",
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
}
