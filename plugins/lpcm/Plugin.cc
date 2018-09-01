#include <util/PluginHelper.h>
using namespace mous;

MOUS_EXPORT_PLUGIN(
    PluginType::Decoder | PluginType::Encoder,
    "lpcm",
    "LPCM Codec",
    2
)
