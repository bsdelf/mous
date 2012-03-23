#include "PlayerStatistics.h"
#include <plugin/PluginHelper.h>

static const PluginInfo info = {
    "Yanhui Shen",
    "Player Statistics",
    "Record player activity & Statistics",
    1
};

MOUS_DEF_PLUGIN(PluginType::EventWatcher, &info, PlayerStatistics);
