#pragma once

#include <unordered_map>

#include <scx/Conv.hpp>
#include <scx/FileHelper.hpp>
using namespace scx;

#include <core/Plugin.h>
#include <plugin/IMediaPack.h>
#include <plugin/ITagParser.h>
#include <util/MediaItem.h>

namespace mous {

class MediaLoader::Impl
{
  public:
    void RegisterMediaPackPlugin(const Plugin* pAgent)
    {
        if (pAgent->Type() == PluginType::MediaPack) {
            AddMediaPack(pAgent);
        }
    }

    void RegisterMediaPackPlugin(std::vector<const Plugin*>& agents)
    {
        for (auto agent : agents) {
            RegisterMediaPackPlugin(agent);
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
            case PluginType::MediaPack:
                RemoveMediaPack(pAgent);
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
        list.reserve(indexedMediaPacks.size());
        for (const auto& entry : indexedMediaPacks) {
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
        // Find MediaPack.
        const std::string& suffix = ToLower(FileHelper::FileSuffix(path));
        auto iter = indexedMediaPacks.find(suffix);

        if (iter == indexedMediaPacks.end()) {
            // General Media
            list.emplace_back(path);
        } else {
            // MediaPack
            IMediaPack* pack = iter->second;
            pack->DumpMedia(path, list, &indexedMediaPacks);
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

    void AddMediaPack(const Plugin* pAgent)
    {
        // Register agent.
        IMediaPack* pPack = (IMediaPack*)pAgent->CreateObject();
        indexedObjects.emplace(pAgent, pPack);

        // Register MediaPack.
        for (const std::string& item : pPack->FileSuffix()) {
            const std::string& suffix = ToLower(item);
            auto iter = indexedMediaPacks.find(suffix);
            if (iter == indexedMediaPacks.end()) {
                indexedMediaPacks.emplace(suffix, pPack);
            }
        }
    }

    void RemoveMediaPack(const Plugin* pAgent)
    {
        auto iter = indexedObjects.find(pAgent);
        if (iter != indexedObjects.end()) {
            // Unregister MediaPack.
            IMediaPack* pPack = (IMediaPack*)iter->second;
            for (const std::string& item : pPack->FileSuffix()) {
                const std::string& suffix = ToLower(item);
                auto iter = indexedMediaPacks.find(suffix);
                if (iter != indexedMediaPacks.end()) {
                    indexedMediaPacks.erase(iter);
                }
            }

            // Unregister Plugin.
            pAgent->FreeObject(pPack);
            indexedObjects.erase(iter);
        }
    }

    void AddTagParser(const Plugin* pAgent)
    {
        // Register Plugin.
        ITagParser* pParser = (ITagParser*)pAgent->CreateObject();
        indexedObjects.emplace(pAgent, pParser);

        // Register TagParser.
        for (const std::string& item : pParser->FileSuffix()) {
            const std::string& suffix = ToLower(item);
            auto iter = indexedTagParsers.find(suffix);
            if (iter == indexedTagParsers.end()) {
                indexedTagParsers.emplace(suffix, pParser);
            }
        }
    }

    void RemoveTagParser(const Plugin* pAgent)
    {
        auto iter = indexedObjects.find(pAgent);
        if (iter != indexedObjects.end()) {
            // Unregister TagParser.
            ITagParser* pParser = (ITagParser*)iter->second;
            for (const std::string& item : pParser->FileSuffix()) {
                const std::string& suffix = ToLower(item);
                auto iter = indexedTagParsers.find(suffix);
                if (iter != indexedTagParsers.end()) {
                    indexedTagParsers.erase(iter);
                }
            }

            // Unregister Plugin.
            pAgent->FreeObject(pParser);
            indexedObjects.erase(iter);
        }
    }

  private:
    std::unordered_map<const Plugin*, void*> indexedObjects;
    std::unordered_map<std::string, IMediaPack*> indexedMediaPacks;
    std::unordered_map<std::string, ITagParser*> indexedTagParsers;

};
}
