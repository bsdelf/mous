#pragma once

#include <unordered_map>

#include <scx/Conv.hpp>
#include <scx/FileHelper.hpp>
using namespace scx;

#include <core/Plugin.h>
#include <plugin/ISheetParser.h>
#include <plugin/ITagParser.h>
#include <util/MediaItem.h>

namespace mous {

class MediaLoader::Impl
{
  public:
    void RegisterSheetParserPlugin(const Plugin* pAgent)
    {
        if (pAgent->Type() == PluginType::SheetParser) {
            AddSheetParser(pAgent);
        }
    }

    void RegisterSheetParserPlugin(std::vector<const Plugin*>& agents)
    {
        for (auto agent : agents) {
            RegisterSheetParserPlugin(agent);
        }
    }

    void RegisterTagParserPlugin(const Plugin* pAgent)
    {
        if (pAgent->Type() == PluginType::TagParser) {
            AddTagParser(pAgent);
        }
    }

    void RegisterTagParserPlugin(std::vector<const Plugin*>& agents)
    {
        for (auto agent : agents) {
            RegisterTagParserPlugin(agent);
        }
    }

    void UnregisterPlugin(const Plugin* pAgent)
    {
        switch (pAgent->Type()) {
            case PluginType::SheetParser:
                RemoveSheetParser(pAgent);
                break;

            case PluginType::TagParser:
                RemoveTagParser(pAgent);
                break;

            default:
                break;
        }
    }

    void UnregisterPlugin(std::vector<const Plugin*>& agents)
    {
        for (auto agent : agents) {
            UnregisterPlugin(agent);
        }
    }

    void UnregisterAll()
    {
        while (!indexedObjects.empty()) {
            auto iter = indexedObjects.begin();
            UnregisterPlugin(iter->first);
        }
    }

    std::vector<std::string> SupportedSuffixes() const
    {
        std::vector<std::string> list;
        list.reserve(indexedSheetParsers.size());
        for (const auto& entry : indexedSheetParsers) {
            list.push_back(entry.first);
        }
        return list;
    }

    ErrorCode LoadMedia(const std::string& path, std::deque<MediaItem>& list) const
    {
        list.clear();

        TryUnpack(path, list);
        TryParseTag(list);

        return ErrorCode::Ok;
    }

  private:
    ErrorCode TryUnpack(const std::string& path, std::deque<MediaItem>& list) const
    {
        // Find SheetParser.
        const std::string& suffix = ToLower(FileHelper::FileSuffix(path));
        auto iter = indexedSheetParsers.find(suffix);

        if (iter == indexedSheetParsers.end()) {
            // General Media
            list.emplace_back(path);
        } else {
            // SheetParser
            ISheetParser* parser = iter->second;
            parser->DumpMedia(path, list);
        }

        return ErrorCode::Ok;
    }

    ErrorCode TryParseTag(std::deque<MediaItem>& list) const
    {
        for (auto& item : list) {
            // Find TagParser.
            const std::string& suffix = ToLower(FileHelper::FileSuffix(item.url));
            auto iter = indexedTagParsers.find(suffix);
            if (iter == indexedTagParsers.end()) {
                iter = indexedTagParsers.find("*");
                if (iter == indexedTagParsers.end()) {
                    continue;
                }
            }

            // Parse & Fill.
            ITagParser* parser = iter->second;
            parser->Open(item.url);
            if (parser->HasTag()) {
                if (item.tag.title.empty()) {
                    item.tag.title = parser->Title();
                }
                if (item.tag.artist.empty()) {
                    item.tag.artist = parser->Artist();
                }
                if (item.tag.album.empty()) {
                    item.tag.album = parser->Album();
                }
                if (item.tag.comment.empty()) {
                    item.tag.comment = parser->Comment();
                }
                if (item.tag.genre.empty()) {
                    item.tag.genre = parser->Genre();
                }
                if (item.tag.year < 0) {
                    item.tag.year = parser->Year();
                }
                if (item.tag.track < 0) {
                    item.tag.track = parser->Track();
                }
            } else {
                // cout << "WARN: no tag!!" << endl;
            }

            if (parser->HasAudioProperty()) {
                if (item.duration < 0) {
                    item.duration = parser->Duration();
                }
            } else {
                item.duration = 0;
                // cout << "FATAL: no properties!!" << endl;
            }

            parser->Close();
        }

        return ErrorCode::Ok;
    }

    void AddSheetParser(const Plugin* pAgent)
    {
        // Register agent.
        ISheetParser* sheetParser = (ISheetParser*)pAgent->CreateObject();
        indexedObjects.emplace(pAgent, sheetParser);

        // Register SheetParser.
        for (const std::string& item : sheetParser->FileSuffix()) {
            const std::string& suffix = ToLower(item);
            auto iter = indexedSheetParsers.find(suffix);
            if (iter == indexedSheetParsers.end()) {
                indexedSheetParsers.emplace(suffix, sheetParser);
            }
        }
    }

    void RemoveSheetParser(const Plugin* pAgent)
    {
        auto iter = indexedObjects.find(pAgent);
        if (iter != indexedObjects.end()) {
            // Unregister SheetParser.
            ISheetParser* sheetParser = (ISheetParser*)iter->second;
            for (const std::string& item : sheetParser->FileSuffix()) {
                const std::string& suffix = ToLower(item);
                auto iter = indexedSheetParsers.find(suffix);
                if (iter != indexedSheetParsers.end()) {
                    indexedSheetParsers.erase(iter);
                }
            }

            // Unregister Plugin.
            pAgent->FreeObject(sheetParser);
            indexedObjects.erase(iter);
        }
    }

    void AddTagParser(const Plugin* pAgent)
    {
        // Register Plugin.
        ITagParser* tagParser = (ITagParser*)pAgent->CreateObject();
        indexedObjects.emplace(pAgent, tagParser);

        // Register TagParser.
        for (const std::string& item : tagParser->FileSuffix()) {
            const std::string& suffix = ToLower(item);
            auto iter = indexedTagParsers.find(suffix);
            if (iter == indexedTagParsers.end()) {
                indexedTagParsers.emplace(suffix, tagParser);
            }
        }
    }

    void RemoveTagParser(const Plugin* pAgent)
    {
        auto iter = indexedObjects.find(pAgent);
        if (iter != indexedObjects.end()) {
            // Unregister TagParser.
            ITagParser* tagParser = (ITagParser*)iter->second;
            for (const std::string& item : tagParser->FileSuffix()) {
                const std::string& suffix = ToLower(item);
                auto iter = indexedTagParsers.find(suffix);
                if (iter != indexedTagParsers.end()) {
                    indexedTagParsers.erase(iter);
                }
            }

            // Unregister Plugin.
            pAgent->FreeObject(tagParser);
            indexedObjects.erase(iter);
        }
    }

  private:
    std::unordered_map<const Plugin*, void*> indexedObjects;
    std::unordered_map<std::string, ISheetParser*> indexedSheetParsers;
    std::unordered_map<std::string, ITagParser*> indexedTagParsers;

};
}
