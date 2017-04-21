#pragma once

#include <unordered_map>

#include <scx/FileHelper.hpp>

namespace mous {

class TagParserFactory::Impl {
public:
    ~Impl()
    {
        UnregisterAll();
    }

    void RegisterTagParserPlugin(const Plugin* pAgent)
    {
        ITagParser* parser = static_cast<ITagParser*>(pAgent->CreateObject());
        const auto& suffixList = parser->FileSuffix();
        if (parser != nullptr) {
            pAgent->FreeObject(parser);
            for (const std::string& suffix: suffixList) {
                auto iter = indexedPlugins.find(suffix);
                if (iter == indexedPlugins.end()) {
                    indexedPlugins.emplace(suffix, pAgent);
                }
            }
        }
    }

    void RegisterTagParserPlugin(std::vector<const Plugin*>& agents)
    {
        for (auto agent: agents) {
            RegisterTagParserPlugin(agent);
        }
    }

    void UnregisterPlugin(const Plugin* pAgent)
    {
        ITagParser* parser = static_cast<ITagParser*>(pAgent->CreateObject());
        const auto& suffixList = parser->FileSuffix();
        if (parser != nullptr) {
            pAgent->FreeObject(parser);
            for (const std::string& suffix: suffixList) {
                auto iter = indexedPlugins.find(suffix);
                if (iter != indexedPlugins.end() && pAgent == iter->second) {
                    indexedPlugins.erase(iter);
                    // we do not care about the TagParser in use, let it leak/crash!
                }
            }
        }
    }

    void UnregisterPlugin(std::vector<const Plugin*>& agents)
    {
        for (auto agent: agents) {
            UnregisterPlugin(agent);
        }
    }

    void UnregisterAll()
    {
        while (!indexedPlugins.empty()) {
            UnregisterPlugin(indexedPlugins.begin()->second);
        }
    }

    ITagParser* CreateParser(const std::string& fileName) const
    {
        ITagParser* parser = nullptr;
        const std::string& suffix = scx::FileHelper::FileSuffix(fileName);
        auto iter = indexedPlugins.find(suffix);
        if (iter == indexedPlugins.end()) {
            iter = indexedPlugins.find("*");
        }
        if (iter != indexedPlugins.end()) {
            parser =  static_cast<ITagParser*>(iter->second->CreateObject());
        }
        if (parser != nullptr) {
            m_ParserParentMap[parser] = iter->second;
        }
        return parser;
    }

    void FreeParser(ITagParser* parser) const
    {
        auto iter = m_ParserParentMap.find(parser);
        if (iter != m_ParserParentMap.end()) {
            iter->second->FreeObject(parser);
            m_ParserParentMap.erase(iter);
        }
    }

private:
    std::unordered_map<std::string, const Plugin*> indexedPlugins;
    mutable std::unordered_map<ITagParser*, const Plugin*> m_ParserParentMap;
};

}
