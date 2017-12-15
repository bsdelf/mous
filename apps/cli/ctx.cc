#include <string>

#include <scx/FileInfo.h>
using namespace scx;

#include "ctx.h"

static const char* const PLUGIN_ROOT = "/lib/mous/";

Context ctx;

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
    pluginManager.LoadPluginDir(pluginDir);

    // get plugin agents and check if we have enough
    decoderPlugins = pluginManager.PluginAgents(PluginType::Decoder);
    encoderPlugins = pluginManager.PluginAgents(PluginType::Encoder);
    outputPlugins = pluginManager.PluginAgents(PluginType::Renderer);
    sheetParserPlugins = pluginManager.PluginAgents(PluginType::SheetParser);
    tagParserPlugins = pluginManager.PluginAgents(PluginType::TagParser);
    if (decoderPlugins.empty() || outputPlugins.empty()) {
        printf("need more plugins!\n");
        exit(EXIT_FAILURE);
    }

    // setup media loader
    loader.RegisterSheetParserPlugin(sheetParserPlugins);
    loader.RegisterTagParserPlugin(tagParserPlugins);
    // setup parser factory
    tagParserFactory.RegisterTagParserPlugin(tagParserPlugins);
    // setup conv factory
    convertTaskFactory.RegisterDecoderPlugin(decoderPlugins);
    convertTaskFactory.RegisterEncoderPlugin(encoderPlugins);
}

Context::~Context()
{
    convertTaskFactory.UnregisterAll();
    tagParserFactory.UnregisterAll();
    loader.UnregisterAll();
    pluginManager.UnloadAll();
}

