#pragma once

#include <vector>

#include <util/MediaItem.h>
#include <util/PluginOption.h>
#include <core/MediaLoader.h>
#include <core/PluginManager.h>
#include <core/ConvTask.h>
#include <core/ConvTaskFactory.h>
#include <core/TagParserFactory.h>
using namespace mous;

struct Context
{
    using PluginPtrVector = std::vector<const Plugin*>;

    Context();
    ~Context();

    PluginManager pluginManager;

    MediaLoader loader;
    TagParserFactory tagParserFactory;
    ConvTaskFactory convertTaskFactory;

    PluginPtrVector decoderPlugins;
    PluginPtrVector encoderPlugins;
    PluginPtrVector outputPlugins;
    PluginPtrVector tagParserPlugins;
    PluginPtrVector sheetParserPlugins;
};

extern Context ctx;
