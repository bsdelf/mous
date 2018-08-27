#include <libcue.h>
#include <cstdio>
#include <scx/FileHelper.h>
using namespace scx;

#include <util/MediaItem.h>
#include <plugin/SheetParserProto.h>
using namespace mous;

namespace {
    struct Self {};
}

static void DumpCue(const std::string& dir, Cd* cd, std::deque<MediaItem>& items);

static void* Create() {
    return new Self();
}

static void Destroy(void* ptr) {
    delete SELF;
}

static void DumpFile(void* ptr, const char* path, std::deque<MediaItem>* items) {
    FILE* file = fopen(path, "r");
    if (!file) return;
    Cd* cd = cue_parse_file(file);
    fclose(file);
    std::string dir = FileHelper::FileDir(path);
    DumpCue(dir, cd, *items);
    cd_delete(cd);
}

static void DumpStream(void* ptr, const char* stream, std::deque<MediaItem>* items) {
    Cd* cd = cue_parse_string(stream);
    DumpCue("", cd, *items);
    cd_delete(cd);
}

static const char** GetSuffixes(void* ptr) {
    static const char* suffixes[] { "cue", nullptr };
    return suffixes;
}

static void DumpCue(const std::string& dir, Cd* cd, std::deque<mous::MediaItem>& items) {
    const auto cdtext_get_str = [](enum Pti pti, const Cdtext* cdt, const std::string& defa) {
        const char* buf = cdtext_get(pti, cdt);
        return buf ? std::string(buf) : defa;
    };

    const auto rem_get_num = [](unsigned int i, Rem* rem, int defa) {
        const char* buf = rem_get(i, rem);
        return buf ? std::stoi(buf) : defa;
    };

    Cdtext* cdt = cd_get_cdtext(cd);
    Rem* rem = cd_get_rem(cd);
    const std::string cd_album = cdtext_get_str(PTI_TITLE, cdt, "");
    const std::string cd_artist = cdtext_get_str(PTI_PERFORMER, cdt, "");
    const std::string cd_genre = cdtext_get_str(PTI_GENRE, cdt, "");
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

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}
