#include <util/PluginHelper.h>
#include "SheetParser.h"

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

    const SheetParserInterface* MousGetSheetParserInterface() {
        return &sheet_parser_interface;
    }
}
