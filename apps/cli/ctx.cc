#include <string>

#include <scx/FileInfo.h>
using namespace scx;

#include <util/PluginFinder.h>

#include "ctx.h"

static const char* const PLUGIN_ROOT = "/lib/mous/";

std::unique_ptr<Context> ctx = nullptr;

Context::Context()
{
    // check plugin path then load it
    FileInfo dir_info(std::string(CMAKE_INSTALL_PREFIX) + PLUGIN_ROOT);
    const std::string pluginDir(dir_info.AbsFilePath());
    if (!dir_info.Exists() 
        || dir_info.Type() != FileType::Directory
        || pluginDir.empty()) {
        printf("bad plugin directory!\n");
        exit(EXIT_FAILURE);
    }

    PluginFinder()
        .OnPlugin(PluginType::FormatProbe, [this](const std::shared_ptr<Plugin>& plugin) {
            player.LoadFormatProbePlugin(plugin);
        })
        .OnPlugin(PluginType::Decoder, [this](const std::shared_ptr<Plugin>& plugin) {
            decoderPlugins.push_back(plugin);
            player.LoadDecoderPlugin(plugin);
            converter.LoadDecoderPlugin(plugin);
        })
        .OnPlugin(PluginType::Encoder, [this](const std::shared_ptr<Plugin>& plugin) {
            encoderPlugins.push_back(plugin);
            converter.LoadEncoderPlugin(plugin);
        })
        .OnPlugin(PluginType::Output, [this](const std::shared_ptr<Plugin>& plugin) {
            outputPlugins.push_back(plugin);
            player.LoadOutputPlugin(plugin);
        })
        .OnPlugin(PluginType::SheetParser, [this](const std::shared_ptr<Plugin>& plugin) {
            sheetParserPlugins.push_back(plugin);
            mediaLoader.LoadSheetParserPlugin(plugin);
        })
        .OnPlugin(PluginType::TagParser, [this](const std::shared_ptr<Plugin>& plugin) {
            tagParserPlugins.push_back(plugin);
            mediaLoader.LoadTagParserPlugin(plugin);
            tagParserFactory.LoadTagParserPlugin(plugin);
        })
        .Run(pluginDir);

    if (decoderPlugins.empty() || outputPlugins.empty()) {
        printf("need more plugins!\n");
        exit(EXIT_FAILURE);
    }
}

Context::~Context()
{
    mediaLoader.UnloadPlugin();
    player.UnloadPlugin();
}

