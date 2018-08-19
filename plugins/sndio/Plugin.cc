#include <util/PluginHelper.h>
#include "Output.h"

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

    const OutputInterface* MousGetOutputInterface() {
        return &output_interface;
    }
}
