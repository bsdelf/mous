#pragma once

#include <vector>
#include <mutex>

#include <util/MediaItem.h>
#include <core/MediaLoader.h>
#include <core/Player.h>
#include <util/Playlist.h>
#include <core/Converter.h>
#include <core/TagParserFactory.h>
using namespace mous;

struct Context
{
    using PluginVector = std::vector<std::shared_ptr<Plugin>>;

    Context();
    ~Context();

    MediaLoader mediaLoader;
    TagParserFactory tagParserFactory;
    Converter converter;

    Player player;
    std::mutex playerMutex;
    bool playerQuit;
    Playlist<MediaItem> playlist;

    PluginVector decoderPlugins;
    PluginVector encoderPlugins;
    PluginVector outputPlugins;
    PluginVector tagParserPlugins;
    PluginVector sheetParserPlugins;
};

extern std::unique_ptr<Context> ctx;
