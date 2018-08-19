#include <util/PluginHelper.h>
#include "Output.h"

static const PluginInfo plugin_info {
    "alsa",
    "ALSA Output",
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

    const OutputInterface* MousGetOutputInterface() {
        return &output_interface;
    }
}