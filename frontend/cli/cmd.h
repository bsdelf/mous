#pragma once

#include <vector>

#include <core/PluginManager.h>
#include <core/MediaLoader.h>
#include <core/TagParserFactory.h>
#include <core/ConvTaskFactory.h>

struct mous_ctx
{
    typedef std::vector<const mous::Plugin*> plugin_list_t;

    mous::PluginManager mgr;

    mous::MediaLoader loader;
    mous::TagParserFactory parser_factory;
    mous::ConvTaskFactory conv_factory;

    plugin_list_t dec_agents;
    plugin_list_t enc_agents;
    plugin_list_t red_agents;
    plugin_list_t tag_parser_agents;
    plugin_list_t sheet_parser_agents;
};

extern mous_ctx ctx;

extern int cmd_play(int, char**);
extern int cmd_dec(int, char**);
extern int cmd_img(int, char**);
extern int cmd_info(int, char**);
extern int cmd_plugin(int, char**);
extern int cmd_help(int, char**);
