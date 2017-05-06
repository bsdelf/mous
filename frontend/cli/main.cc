#include <assert.h>
#include <unistd.h> // for usleep()
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <vector>
#include <string>
using namespace std;

#include <util/MediaItem.h>
#include <util/PluginOption.h>
#include <core/MediaLoader.h>
#include <core/PluginManager.h>
#include <core/ConvTask.h>
#include <core/ConvTaskFactory.h>
using namespace mous;

#include <scx/FileInfo.hpp>
using namespace scx;

#include "cmd.h"

namespace Path {
    const char* const PluginRoot = "/lib/mous/";
};

mous_ctx ctx;

struct cmd_action_t
{
    const char* cmd;
    int (*action)(int, char**);
};

static cmd_action_t cmd_actions[] = {
    {   "play",   cmd_play      },
    {   "dec",    cmd_dec       },
    {   "img",    cmd_img       },
    {   "info",   cmd_info      },
    {   "plugin", cmd_plugin    },
    {   "help",   cmd_help      }
};

static const char* cli_name = nullptr;

int cmd_help(int, char**)
{
    printf("Usage: %s <command> <options> <files>\n"
            "play   (default command)\n"
            "       -r(repeat) -s(shuffle)\n"
            "dec    (decode to wav)\n"
            "img    (dump cover art)\n"
            "info   (display file information)\n"
            "plugin (display plugin information)\n"
            "help   (display help information)\n", cli_name);

    return 0;
}

int cmd_plugin(int, char**)
{
    const vector<string>& path_list = ctx.mgr.PluginPaths();
    for (size_t i = 0; i < path_list.size(); ++i) {
        const PluginInfo* info = ctx.mgr.QueryPluginInfo(path_list[i]);
        printf("#%02zu %s\n"
               "    %s\n"
               "    %s\n"
               "    %s\n"
               "    %d\n",
               i+1, path_list[i].c_str(),
               info->name,
               info->desc,
               info->author,
               info->version);
    }

    printf("Decoders:   %zu\n"
           "Endocers:   %zu\n"
           "Renderers:  %zu\n"
           "MediaPacks: %zu\n"
           "TagParsers: %zu\n",
           ctx.dec_agents.size(),
           ctx.enc_agents.size(),
           ctx.red_agents.size(),
           ctx.pack_agents.size(),
           ctx.tag_agents.size());

    return 0;
}

int main(int argc, char** argv)
{
    int ret = 0;
    cli_name = argc > 0 ? argv[0] : "mous-cli";

    if (argc < 2) {
        cmd_help(0, nullptr);
        exit(-1);
    }

    // check plugin path then load it
    FileInfo dir_info(string(CMAKE_INSTALL_PREFIX) + Path::PluginRoot);
    const string pluginDir(dir_info.AbsFilePath());
    if (!dir_info.Exists() 
        || dir_info.Type() != FileType::Directory
        || pluginDir.empty()) {
        printf("bad plugin directory!\n");
        exit(-1);
    }
    ctx.mgr.LoadPluginDir(pluginDir);

    // get plugin agents and check if we have enough
    ctx.dec_agents = ctx.mgr.PluginAgents(PluginType::Decoder);
    ctx.enc_agents = ctx.mgr.PluginAgents(PluginType::Encoder);
    ctx.red_agents = ctx.mgr.PluginAgents(PluginType::Renderer);
    ctx.pack_agents = ctx.mgr.PluginAgents(PluginType::MediaPack);
    ctx.tag_agents = ctx.mgr.PluginAgents(PluginType::TagParser);
    if (ctx.dec_agents.empty() || ctx.red_agents.empty()) {
        printf("need more plugins!\n");
        cmd_plugin(0, nullptr);
        exit(-1);
    }

    // setup media loader
    ctx.loader.RegisterMediaPackPlugin(ctx.pack_agents);
    ctx.loader.RegisterTagParserPlugin(ctx.tag_agents);
    // setup parser factory
    ctx.parser_factory.RegisterTagParserPlugin(ctx.tag_agents);
    // setup conv factory
    ctx.conv_factory.RegisterDecoderPlugin(ctx.dec_agents);
    ctx.conv_factory.RegisterEncoderPlugin(ctx.enc_agents);

    // match command
    {
        const size_t len = strlen(argv[1]);
        const int count = sizeof(cmd_actions)/sizeof(cmd_action_t);
        int index = 0;
        for (; index < count; ++index) {
            if (strncmp(cmd_actions[index].cmd, argv[1], len) == 0) {
                ret = cmd_actions[index].action(argc-1, argv+1);
                break;
            }
        }

        // "play" is the default cmd
        if (index == count) {
            ret = cmd_play(argc, argv);
        }
    }

    // cleanup
    ctx.conv_factory.UnregisterAll();
    ctx.parser_factory.UnregisterAll();
    ctx.loader.UnregisterAll();

    ctx.mgr.UnloadAll();

    exit(ret);
}

/*
void PrintPluginOption(vector<PluginOption>& list)
{
    for (size_t i = 0; i < list.size(); ++i) {
        PluginOption& opt = list[i];
        cout << ">>>> index:" << i+1 << endl;
        cout << "\tplugin type: " << ToString(opt.pluginType)<< endl;
        for (size_t io = 0; io < opt.options.size(); ++io) {
            cout << "\t\t option type: " << ToString(opt.options[io]->type) << endl;
            cout << "\t\t option desc: " << opt.options[io]->desc << endl;
        }
    }
}
*/

    // Show player options 
    /*
    {
        vector<PluginOption> list;
        cout << ">> Player decoder plugin options:" << endl;
        player->DecoderPluginOption(list);
        PrintPluginOption(list);
        cout << ">> Player renderer plugin options:" << endl;
        PluginOption opt;
        player->RendererPluginOption(opt);
        list.resize(1);
        list[0] = opt;
        PrintPluginOption(list);
    }
    */


