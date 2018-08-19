#include <util/PluginHelper.h>
#include "TagParser.h"

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

    const TagParserInterface* MousGetTagParserInterface() {
        return &tag_parser_interface;
    }
}

