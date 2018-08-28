#include <util/PluginHelper.h>
using namespace mous;

MOUS_EXPORT_PLUGIN(
    PluginType::Decoder | PluginType::Encoder,
    "wav",
    "WAV Codec",
    2
)
