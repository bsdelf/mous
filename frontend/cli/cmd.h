#ifndef _MOUS_CLI_CMD_H_
#define _MOUS_CLI_CMD_H_

#include <vector>

#include <core/IPluginManager.h>
#include <core/IMediaLoader.h>

struct mous_ctx
{
    typedef std::vector<const mous::IPluginAgent*> plugin_list_t;

    mous::IPluginManager* mgr;
    mous::IMediaLoader* loader;

    plugin_list_t dec_agents;
    plugin_list_t enc_agents;
    plugin_list_t red_agents;
    plugin_list_t tag_agents;
    plugin_list_t pack_agents;
};

extern mous_ctx ctx;

extern int cmd_help(int, char**);
extern int cmd_plugin(int, char**);
extern int cmd_dec(int, char**);
extern int cmd_play(int, char**);

#endif
