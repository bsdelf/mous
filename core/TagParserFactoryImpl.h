#pragma once

#include <map>
#include <scx/FileHelper.h>

namespace mous {

class TagParserFactory::Impl {
public:
    ~Impl()
    {
        UnloadPlugin();
    }

    void LoadTagParserPlugin(const std::shared_ptr<Plugin>& plugin)
    {
        auto parser = plugin->CreateObject<ITagParser*>();
        if (!parser) {
            return;
        }
        const auto& suffixList = parser->FileSuffix();
        for (const std::string& suffix: suffixList) {
            auto iter = plugins_.find(suffix);
            if (iter == plugins_.end()) {
                plugins_.emplace(suffix, plugin);
            }
        }
        plugin->FreeObject(parser);
    }

    void UnloadPlugin(const std::string&)
    {
        // TODO
    }

    void UnloadPlugin()
    {
        for (auto& kv: parsers_) {
            kv.second->FreeObject(kv.first);
        }
        parsers_.clear();
        plugins_.clear();
    }

    ITagParser* CreateParser(const std::string& fileName) const
    {
        const std::string& suffix = scx::FileHelper::FileSuffix(fileName);
        auto iter = plugins_.find(suffix);
        if (iter == plugins_.end()) {
            iter = plugins_.find("*");
        }
        if (iter == plugins_.end()) {
            return nullptr;
        }
        auto parser = iter->second->CreateObject<ITagParser*>();
        if (parser != nullptr) {
            parsers_[parser] = iter->second;
        }
        return parser;
    }

    void FreeParser(ITagParser* parser) const
    {
        auto iter = parsers_.find(parser);
        if (iter != parsers_.end()) {
            iter->second->FreeObject(parser);
            parsers_.erase(iter);
        }
    }

private:
    std::map<std::string, std::shared_ptr<Plugin>> plugins_;
    mutable std::map<ITagParser*, std::shared_ptr<Plugin>> parsers_;
};

}
