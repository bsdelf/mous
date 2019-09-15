#pragma once

#include <mutex>
#include <vector>

#include <core/Converter.h>
#include <core/MediaLoader.h>
#include <core/Player.h>
#include <core/TagParserFactory.h>
#include <util/MediaItem.h>
#include <util/Playlist.h>
using namespace mous;

struct Context {
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
