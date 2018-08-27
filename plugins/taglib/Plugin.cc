#include <util/PluginHelper.h>
using namespace mous;

static const PluginInfo info = {
    "taglib",
    "TagLib is for reading and editing the meta-data",
    2
};

extern "C" {
    PluginType MousGetPluginType() {
        return PluginType::TagParser;
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

