#ifndef MOUS_PLUGINHELPER
#define MOUS_PLUGINHELPER

/**
 * Plugin common definition.
 */
namespace mous {

enum PluginType {
    MousNone = 0,
    MousDecoder,
    MousEncoder,
    MousRenderer,
    MousMediaList,
    MousFilter
};

struct PluginInfo {
    const char* author;
    const char* name;
    const char* description;
    const int32_t version;
};

const char* const StrGetPluginType = "GetPluginType";
const char* const StrGetPluginInfo = "GetPluginInfo";
const char* const StrCreatePlugin = "CreatePlugin";
const char* const StrReleasePlugin = "ReleasePlugin";

}

/**
 * Simple yet helpful macro for declare a plugin.
 */
#define MOUS_DEF_PLUGIN(type, pInfo, Super, Sub)	\
extern "C" {						\
    PluginType GetPluginType() {			\
	return type;					\
    }							\
    \
    const PluginInfo* GetPluginInfo() {			\
	return pInfo;					\
    }							\
    \
    Super* CreatePlugin() {				\
	return new Sub;					\
    }							\
    \
    void ReleasePlugin(Super* p) {			\
	if (p != NULL)					\
	    delete p;					\
    }							\
} struct __end__

#endif
