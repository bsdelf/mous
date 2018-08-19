#pragma once

#include <map>
#include <memory>

#include <scx/Conv.h>
#include <scx/FileHelper.h>
using namespace scx;

#include <util/Plugin.h>
#include <plugin/SheetParser.h>
#include <plugin/TagParser.h>
#include <util/MediaItem.h>

namespace mous {

class MediaLoader::Impl
{
  public:
    void LoadSheetParserPlugin(const std::shared_ptr<Plugin>& plugin)
    {
        auto parser = std::make_shared<SheetParser>(plugin);
        if (!parser || !*parser) {
            return;
        }
        for (const std::string& item: parser->FileSuffix()) {
            const std::string& suffix = ToLower(item);
            auto iter = sheet_parsers_.find(suffix);
            if (iter == sheet_parsers_.end()) {
                sheet_parsers_.emplace(suffix, parser);
            }
        }
    }

    void LoadTagParserPlugin(const std::shared_ptr<Plugin>& plugin)
    {
        auto parser = std::make_shared<TagParser>(plugin);
        if (!parser || !*parser) {
            return;
        }
        for (const std::string& item: parser->FileSuffix()) {
            const std::string& suffix = ToLower(item);
            auto iter = tag_parsers_.find(suffix);
            if (iter == tag_parsers_.end()) {
                tag_parsers_.emplace(suffix, parser);
            }
        }
    }

    void UnloadPlugin(const std::string&)
    {
        // TODO
    }

    void UnloadPlugin()
    {
        sheet_parsers_.clear();
        tag_parsers_.clear();
    }

    std::vector<std::string> SupportedSuffixes() const
    {
        std::vector<std::string> list;
        list.reserve(sheet_parsers_.size());
        for (const auto& kv: sheet_parsers_) {
            list.push_back(kv.first);
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
        const std::string& suffix = ToLower(FileHelper::FileSuffix(path));
        const auto iter = sheet_parsers_.find(suffix);
        if (iter == sheet_parsers_.end()) {
            list.emplace_back(path);
        } else {
            const auto& parser = iter->second;
            parser->DumpFile(path, list);
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
            const auto& parser = iter->second;
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

            if (parser->HasAudioProperties()) {
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
    std::map<std::string, std::shared_ptr<SheetParser>> sheet_parsers_;
    std::map<std::string, std::shared_ptr<TagParser>> tag_parsers_;
};

}
