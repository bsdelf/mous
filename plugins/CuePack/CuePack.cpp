#include "CuePack.h"
#include <util/MediaItem.h>
#include <scx/FileHelper.hpp>
#include <scx/Conv.hpp>
#include <cstdio>
#include <iostream>
using namespace scx;
using namespace std;

CuePack::CuePack()
{

}

CuePack::~CuePack()
{

}

vector<string> CuePack::FileSuffix() const
{
    vector<string> list;
    list.clear();
    list.push_back("cue");
    return list;
}

void CuePack::DumpMedia(const std::string& path, std::deque<MediaItem>& list,
    const std::map<std::string, IMediaPack*>* pMap) const
{
    FILE* file = fopen(path.c_str(), "r");
    Cd* cd = cue_parse_file(file);
    fclose(file);

    string dir = FileHelper::FileDir(path);

    DumpCue(dir, cd, list);
}

void CuePack::DumpStream(const std::string& stream, std::deque<MediaItem>& list,
    const std::map<std::string, IMediaPack*>* pMap) const
{
    Cd* cd = cue_parse_string(stream.c_str());
    DumpCue("", cd, list);
}

void CuePack::DumpCue(const string& dir, Cd* cd, deque<MediaItem>& list) const
{
    int ntrack = cd_get_ntrack(cd);

    string album;
    string artist;
    string genre;
    int year = -1;

    char* data = NULL;
    Cdtext* cdt = cd_get_cdtext(cd);
    Rem* rem = cd_get_rem(cd);

    data = cdtext_get(PTI_TITLE, cdt);
    if (data != NULL) {
        album = data;
        delete data;
    }

    data = cdtext_get(PTI_PERFORMER, cdt);
    if (data != NULL) {
        artist = data;
        delete data;
    }

    data = cdtext_get(PTI_GENRE, cdt);
    if (data != NULL) {
        genre = data;
        delete data;
    }

    //cdtext_delete(cdt);
    //rem_free(rem);

    data = rem_get(REM_DATE, rem);
    if (data != NULL) {
        year = StrToNum<int>(data);
        delete data;
    }

    list.resize(ntrack);
    for (int i = 1; i <= ntrack; ++i) {
        MediaItem& item = list[i-1];

        Track* track = cd_get_track(cd, i);
        item.url = dir + track_get_filename(track);
        item.hasRange = true;
        //item->msBeg = (track_get_start(track))/75*1000;
        //item->msEnd = item->msBeg + ((uint64_t)track_get_length(track))/75*1000;
        item.msBeg = ((track_get_start(track)
                    //+ track_get_index(track, 1)
                    - track_get_zero_pre(track)) * 1000) / 75;
        item.msEnd = ((track_get_start(track) + track_get_length(track)
                    //- track_get_index(track, 1)
                    + track_get_zero_pre(track)) * 1000) / 75;
        if (item.msBeg >= item.msEnd || i == ntrack)
            item.msEnd = -1;

        Cdtext* text = track_get_cdtext(track);

        item.tag.album = album;
        item.tag.year = year;

        data = cdtext_get(PTI_TITLE, text);
        if (data != NULL) {
            item.tag.title = data;
            delete data;
        }

        data = cdtext_get(PTI_PERFORMER, text);
        if (data != NULL) {
            item.tag.artist = data;
            delete data;
        } else {
            item.tag.artist = artist;
        }

        data = cdtext_get(PTI_GENRE, text);
        if (data != NULL) {
            item.tag.genre = data;
            delete data;
        } else {
            item.tag.genre = genre;
        }

        item.tag.track = i;

        //cdtext_delete(text);

        cout << i << '\t' << item.url << endl;
        cout << "range:" << item.msBeg << "-" << item.msEnd << endl;
    }
}
