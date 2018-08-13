#pragma once

#include <map>

#include <scx/Conv.h>
#include <scx/FileHelper.h>
using namespace scx;

#include <util/Plugin.h>
#include <plugin/ISheetParser.h>
#include <plugin/ITagParser.h>
#include <util/MediaItem.h>

namespace mous {

class MediaLoader::Impl
{
  public:
    void LoadSheetParserPlugin(const std::shared_ptr<Plugin>& plugin)
    {
        auto parser = plugin->CreateObject<ISheetParser*>();
        if (!parser) {
            return;
        }
        for (const std::string& item : parser->FileSuffix()) {
            const std::string& suffix = ToLower(item);
            auto iter = indexedSheetParsers.find(suffix);
            if (iter == indexedSheetParsers.end()) {
                indexedSheetParsers.emplace(suffix, parser);
            }
        }
        plugins_.push_back(plugin);
    }

    void LoadTagParserPlugin(const std::shared_ptr<Plugin>& plugin)
    {
        auto parser = plugin->CreateObject<ITagParser*>();
        if (!parser) {
            return;
        }
        for (const std::string& item : parser->FileSuffix()) {
            const std::string& suffix = ToLower(item);
            auto iter = indexedTagParsers.find(suffix);
            if (iter == indexedTagParsers.end()) {
                indexedTagParsers.emplace(suffix, parser);
            }
        }
        plugins_.push_back(plugin);
    }

    void UnloadPlugin(const std::string&)
    {
        // TODO
    }

    void UnloadPlugin()
    {
        // FIX: leak
        indexedSheetParsers.clear();
        indexedTagParsers.clear();
        plugins_.clear();
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

  private:
    std::vector<std::shared_ptr<Plugin>> plugins_;
    std::map<std::string, ISheetParser*> indexedSheetParsers;
    std::map<std::string, ITagParser*> indexedTagParsers;

};
}
