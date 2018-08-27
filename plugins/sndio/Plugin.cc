#include <util/PluginHelper.h>
using namespace mous;

static const PluginInfo info {
    "sndio",
    "Sndio Output",
    2
};

extern "C" {
    PluginType MousGetPluginType() {
        return PluginType::Output;
    }

    const PluginInfo* MousGetPluginInfo() {
        return &info;
    }

    void* MousCreateObject() {
        return nullptr;
    }

    void MousFreeObject() {
    }
}
