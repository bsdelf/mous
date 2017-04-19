#include <unordered_map>

#include "core/MediaLoader.h"

#include <scx/FileHelper.hpp>
#include <scx/Conv.hpp>
using namespace scx;

#include "plugin/IMediaPack.h"
#include "util/MediaItem.h"
#include "plugin/ITagParser.h"
#include "core/Plugin.h"

namespace mous {

class MediaLoaderPrivate
{
public:
    std::unordered_map<const Plugin*, void*> indexedObjects;
    std::unordered_map<std::string, IMediaPack*> indexedMediaPacks;
    std::unordered_map<std::string, ITagParser*> indexedTagParsers;
};

MediaLoader::MediaLoader()
    : d(std::make_unique<MediaLoaderPrivate>())
{
}

MediaLoader::~MediaLoader()
{
}

void MediaLoader::RegisterMediaPackPlugin(const Plugin* pAgent)
{
    if (pAgent->Type() == PluginType::MediaPack) {
        AddMediaPack(pAgent);
    }
}

void MediaLoader::RegisterMediaPackPlugin(vector<const Plugin*>& agents)
{
    for (auto agent: agents) {
        RegisterMediaPackPlugin(agent);
    }
}

void MediaLoader::RegisterTagParserPlugin(const Plugin* pAgent)
{
    if (pAgent->Type() == PluginType::TagParser) {
        AddTagParser(pAgent);
    }
}

void MediaLoader::RegisterTagParserPlugin(vector<const Plugin*>& agents)
{
    for (auto agent: agents) {
        RegisterTagParserPlugin(agent);
    }
}

void MediaLoader::UnregisterPlugin(const Plugin* pAgent)
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

void MediaLoader::UnregisterPlugin(vector<const Plugin*>& agents)
{
    for (auto agent: agents) {
        UnregisterPlugin(agent);
    }
}

void MediaLoader::UnregisterAll()
{
    while (!d->indexedObjects.empty()) {
        auto iter = d->indexedObjects.begin();
        UnregisterPlugin(iter->first);
    }
}

void MediaLoader::AddMediaPack(const Plugin* pAgent)
{
    // Register agent.
    IMediaPack* pPack = (IMediaPack*)pAgent->CreateObject();
    d->indexedObjects.emplace(pAgent, pPack);

    // Register MediaPack.
    for (const string& item: pPack->FileSuffix()) {
        const string& suffix = ToLower(item);
        auto iter = d->indexedMediaPacks.find(suffix);
        if (iter == d->indexedMediaPacks.end()) {
            d->indexedMediaPacks.emplace(suffix, pPack);
        }
    }
}

void MediaLoader::RemoveMediaPack(const Plugin* pAgent)
{
    auto iter = d->indexedObjects.find(pAgent);
    if (iter != d->indexedObjects.end()) {
        // Unregister MediaPack.
        IMediaPack* pPack = (IMediaPack*)iter->second;
        for (const string& item: pPack->FileSuffix()) {
            const string& suffix = ToLower(item);
            auto iter = d->indexedMediaPacks.find(suffix);
            if (iter != d->indexedMediaPacks.end()) {
                d->indexedMediaPacks.erase(iter);
            }
        }

        // Unregister Plugin.
        pAgent->FreeObject(pPack);
        d->indexedObjects.erase(iter);
    }
}

void MediaLoader::AddTagParser(const Plugin* pAgent)
{
    // Register Plugin.
    ITagParser* pParser = (ITagParser*)pAgent->CreateObject();
    d->indexedObjects.emplace(pAgent, pParser);

    // Register TagParser.
    for (const string& item: pParser->FileSuffix()) {
        const string& suffix = ToLower(item);
        auto iter = d->indexedTagParsers.find(suffix);
        if (iter == d->indexedTagParsers.end()) {
            d->indexedTagParsers.emplace(suffix, pParser);
        }
    }
}

void MediaLoader::RemoveTagParser(const Plugin* pAgent)
{
    auto iter = d->indexedObjects.find(pAgent);
    if (iter != d->indexedObjects.end()) {
        // Unregister TagParser.
        ITagParser* pParser = (ITagParser*)iter->second;
        for (const string& item: pParser->FileSuffix()) {
            const string& suffix = ToLower(item);
            auto iter = d->indexedTagParsers.find(suffix);
            if (iter != d->indexedTagParsers.end()) {
                d->indexedTagParsers.erase(iter);
            }
        }

        // Unregister Plugin.
        pAgent->FreeObject(pParser);
        d->indexedObjects.erase(iter);
    }
}

vector<string> MediaLoader::SupportedSuffixes() const
{
    vector<string> list;
    list.reserve(d->indexedMediaPacks.size());
    for (const auto& entry: d->indexedMediaPacks) {
        list.push_back(entry.first);
    }
    return list;
}

EmErrorCode MediaLoader::LoadMedia(const string& path, deque<MediaItem>& list) const
{
    list.clear();

    TryUnpack(path, list);
    TryParseTag(list);

    return ErrorCode::Ok;
}

EmErrorCode MediaLoader::TryUnpack(const string& path, deque<MediaItem>& list) const
{
    // Find MediaPack.
    const string& suffix = ToLower(FileHelper::FileSuffix(path));
    auto iter = d->indexedMediaPacks.find(suffix);

    if (iter == d->indexedMediaPacks.end()) {
        // General Media
        list.emplace_back(path);
    } else {
        // MediaPack
        IMediaPack* pack = iter->second;
        pack->DumpMedia(path, list, &d->indexedMediaPacks);
    }

    return ErrorCode::Ok;
}

EmErrorCode MediaLoader::TryParseTag(deque<MediaItem>& list) const
{
    for (auto& item: list) {
        // Find TagParser.
        const string& suffix = ToLower(FileHelper::FileSuffix(item.url));
        auto iter = d->indexedTagParsers.find(suffix);
        if (iter == d->indexedTagParsers.end()) {
            iter = d->indexedTagParsers.find("*");
            if (iter == d->indexedTagParsers.end()) {
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
            //cout << "WARN: no tag!!" << endl;
        }

        if (parser->HasAudioProperty()) {
            if (item.duration < 0) {
                item.duration = parser->Duration();
            }
        } else {
            item.duration = 0;
            //cout << "FATAL: no properties!!" << endl;
        }

        parser->Close();
    }

    return ErrorCode::Ok;
}

}
