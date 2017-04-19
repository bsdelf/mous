#ifndef _MOUS_CLI_CMD_H_
#define _MOUS_CLI_CMD_H_

#include <vector>

#include "core/PluginManager.h"
#include "core/MediaLoader.h"
#include "core/ITagParserFactory.h"
#include "core/IConvTaskFactory.h"

struct mous_ctx
{
    typedef std::vector<const mous::Plugin*> plugin_list_t;

    mous::PluginManager mgr;

    mous::MediaLoader loader;
    mous::ITagParserFactory* parser_factory;
    mous::IConvTaskFactory* conv_factory;

    plugin_list_t dec_agents;
    plugin_list_t enc_agents;
    plugin_list_t red_agents;
    plugin_list_t tag_agents;
    plugin_list_t pack_agents;
};

extern mous_ctx ctx;

extern int cmd_play(int, char**);
extern int cmd_dec(int, char**);
extern int cmd_img(int, char**);
extern int cmd_info(int, char**);
extern int cmd_plugin(int, char**);
extern int cmd_help(int, char**);

#endif
