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
        size_t refcnt = 0;
        for (const std::string& item: parser->FileSuffix()) {
            const std::string& suffix = ToLower(item);
            auto iter = sheet_parsers_.find(suffix);
            if (iter == sheet_parsers_.end()) {
                sheet_parsers_.emplace(suffix, std::make_pair(parser, plugin));
                ++refcnt;
            }
        }
        if (!refcnt) {
            plugin->FreeObject(parser);
        }
    }

    void LoadTagParserPlugin(const std::shared_ptr<Plugin>& plugin)
    {
        auto parser = plugin->CreateObject<ITagParser*>();
        if (!parser) {
            return;
        }
        size_t refcnt = 0;
        for (const std::string& item: parser->FileSuffix()) {
            const std::string& suffix = ToLower(item);
            auto iter = tag_parsers_.find(suffix);
            if (iter == tag_parsers_.end()) {
                tag_parsers_.emplace(suffix, std::make_pair(parser, plugin));
                ++refcnt;
            }
        }
        if (!refcnt) {
            plugin->FreeObject(parser);
        }
    }

    void UnloadPlugin(const std::string&)
    {
        // TODO
    }

    void UnloadPlugin()
    {
        for (const auto& kv: sheet_parsers_) {
            const auto& pair = kv.second;
            pair.second->FreeObject(pair.first);
        }
        sheet_parsers_.clear();
        for (const auto& kv: tag_parsers_) {
            const auto& pair = kv.second;
            pair.second->FreeObject(pair.first);
        }
        tag_parsers_.clear();
    }

    std::vector<std::string> SupportedSuffixes() const
    {
        std::vector<std::string> list;
        list.reserve(sheet_parsers_.size());
        for (const auto& entry : sheet_parsers_) {
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
        auto iter = sheet_parsers_.find(suffix);

        if (iter == sheet_parsers_.end()) {
            // General Media
            list.emplace_back(path);
        } else {
            // SheetParser
            ISheetParser* parser = iter->second.first;
            parser->DumpMedia(path, list);
        }

        return ErrorCode::Ok;
    }

    ErrorCode TryParseTag(std::deque<MediaItem>& list) const
    {
        for (auto& item : list) {
            // Find TagParser.
            const std::string& suffix = ToLower(FileHelper::FileSuffix(item.url));
            auto iter = tag_parsers_.find(suffix);
            if (iter == tag_parsers_.end()) {
                iter = tag_parsers_.find("*");
                if (iter == tag_parsers_.end()) {
                    continue;
                }
            }

            // Parse & Fill.
            ITagParser* parser = iter->second.first;
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
    template <typename T>
    using SuffixMap = std::map<std::string, std::pair<T, std::shared_ptr<Plugin>>>;

    SuffixMap<ISheetParser*> sheet_parsers_;
    SuffixMap<ITagParser*> tag_parsers_;
};
}
