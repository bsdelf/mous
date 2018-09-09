#include <util/PluginHelper.h>
using namespace mous;

MOUS_EXPORT_PLUGIN(
    PluginType::Output | PluginType::Decoder,
    "coreaudio",
    "Core Audio Codec & Output",
    2
)
