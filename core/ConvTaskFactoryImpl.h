#pragma once

#include <map>

#include <core/ConvTaskFactory.h>
#include <core/ConvTask.h>
#include <util/Plugin.h>
#include <plugin/Decoder.h>
#include <plugin/Encoder.h>
#include <util/MediaItem.h>

#include <scx/Conv.h>
#include <scx/FileHelper.h>

namespace mous {

class ConvTaskFactory::Impl {
public:
    ~Impl()
    {
        UnloadPlugin();
    }

    void LoadDecoderPlugin(const std::shared_ptr<Plugin>& plugin)
    {
        auto decoder = std::make_unique<Decoder>(plugin);
        if (!decoder || !*decoder) {
            return;
        }
        const auto& list = decoder->FileSuffix();
        for (const std::string& suffix: list) {
            auto iter = decoderPlugins_.find(suffix);
            if (iter == decoderPlugins_.end()) {
                decoderPlugins_.emplace(suffix, plugin);
            }
        }
    }

    void LoadEncoderPlugin(const std::shared_ptr<Plugin>& plugin)
    {
        encoderPlugins_.emplace(plugin->Name(), plugin);
    }

    void UnloadPlugin(const std::string& path)
    {
        // TODO
    }

    void UnloadPlugin()
    {
        decoderPlugins_.clear();
        encoderPlugins_.clear();
    }

    std::vector<std::string> EncoderNames() const
    {
        std::vector<std::string> list;
        list.reserve(encoderPlugins_.size());
        for (const auto& kv: encoderPlugins_) {
            const auto& plugin = kv.second;
            list.push_back(plugin->Name());
        }
        return list;
    }

    ConvTask* CreateTask(const MediaItem& item, const std::string& encoder) const
    {
        const std::string& suffix = scx::ToLower(scx::FileHelper::FileSuffix(item.url));
        auto decoderPluginIter = decoderPlugins_.find(suffix);
        if (decoderPluginIter == decoderPlugins_.end()) {
            return nullptr;
        }
        auto encoderPluginIter = encoderPlugins_.find(encoder);
        if (encoderPluginIter == encoderPlugins_.end()) {
           return nullptr;
        }
        return new ConvTask(item, decoderPluginIter->second, encoderPluginIter->second);
    }

private:
    std::map<std::string, std::shared_ptr<Plugin>> decoderPlugins_;
    std::map<std::string, std::shared_ptr<Plugin>> encoderPlugins_;
};

}
