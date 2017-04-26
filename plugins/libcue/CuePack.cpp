#include "CuePack.h"

#include <cstdio>

#include <scx/FileHelper.hpp>
using namespace scx;

#include "util/MediaItem.h"

CuePack::CuePack()
{

}

CuePack::~CuePack()
{

}

vector<string> CuePack::FileSuffix() const
{
    return { "cue" };
}

void CuePack::DumpMedia(const std::string& path, std::deque<MediaItem>& items,
    const std::unordered_map<std::string, IMediaPack*>* pMap) const
{
    FILE* file = fopen(path.c_str(), "r");
    if (!file) return;
    Cd* cd = cue_parse_file(file);
    fclose(file);
    string dir = FileHelper::FileDir(path);
    DumpCue(dir, cd, items);
    cd_delete(cd);
}

void CuePack::DumpStream(const std::string& stream, std::deque<MediaItem>& items,
    const std::unordered_map<std::string, IMediaPack*>* pMap) const
{
    Cd* cd = cue_parse_string(stream.c_str());
    DumpCue("", cd, items);
    cd_delete(cd);
}

void CuePack::DumpCue(const string& dir, Cd* cd, deque<MediaItem>& items) const
{
    const auto cdtext_get_str = [](enum Pti pti, const Cdtext* cdt, const string& defa) {
        const char* buf = cdtext_get(pti, cdt);
        return buf ? string(buf) : defa;
    };

    const auto rem_get_num = [](unsigned int i, Rem* rem, int defa) {
        const char* buf = rem_get(i, rem);
        return buf ? std::stoi(buf) : defa;
    };

    Cdtext* cdt = cd_get_cdtext(cd);
    Rem* rem = cd_get_rem(cd);
    const string cd_album = cdtext_get_str(PTI_TITLE, cdt, "");
    const string cd_artist = cdtext_get_str(PTI_PERFORMER, cdt, "");
    const string cd_genre = cdtext_get_str(PTI_GENRE, cdt, "");
    const int cd_year = rem_get_num(REM_DATE, rem, -1);

    const int ntrack = cd_get_ntrack(cd);
    items.resize(ntrack);
    for (int i = 1; i <= ntrack; ++i) {
        auto& item = items[i-1];
        auto& item_tag = item.tag;
        Track* track = cd_get_track(cd, i);

        item.url = dir + track_get_filename(track);
        item.hasRange = true;
        // TODO: take care of length <= 0
        {
            const auto start = track_get_start(track);
            const auto length = track_get_length(track);
            const auto zero_pre = track_get_zero_pre(track);
            item.msBeg = (start - zero_pre) * 1000 / 75;
            item.msEnd = (start + length + zero_pre) * 1000 / 75;
            if (item.msBeg >= item.msEnd || i == ntrack) item.msEnd = -1;
        }

        Cdtext* cdt = track_get_cdtext(track);
        item_tag.album = cd_album;
        item_tag.year = cd_year;
        item_tag.title = cdtext_get_str(PTI_TITLE, cdt, "");
        item_tag.artist = cdtext_get_str(PTI_PERFORMER, cdt, cd_artist);
        item_tag.genre = cdtext_get_str(PTI_GENRE, cdt, cd_genre);
        item_tag.track = i;
    }
}
