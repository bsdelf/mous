#include <util/PluginHelper.h>
using namespace mous;

static const PluginInfo plugin_info = {
    "libcue",
    "Cue Sheet Parser",
    2
};

extern "C" {
    PluginType MousGetPluginType() {
        return PluginType::SheetParser;
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
