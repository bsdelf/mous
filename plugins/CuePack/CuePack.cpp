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

    MediaItem tmpItem;
    for (int i = 1; i <= ntrack; ++i) {
        list.push_back(tmpItem);

        Track* track = cd_get_track(cd, i);
        tmpItem.url = dir + track_get_filename(track);
        tmpItem.hasRange = true;
        //item->msBeg = (track_get_start(track))/75*1000;
        //item->msEnd = item->msBeg + ((uint64_t)track_get_length(track))/75*1000;
        tmpItem.msBeg = ((track_get_start(track)
                    //+ track_get_index(track, 1)
                    - track_get_zero_pre(track)) * 1000) / 75;
        tmpItem.msEnd = ((track_get_start(track) + track_get_length(track)
                    //- track_get_index(track, 1)
                    + track_get_zero_pre(track)) * 1000) / 75;
        if (tmpItem.msBeg >= tmpItem.msEnd || i == ntrack)
            tmpItem.msEnd = -1;

        Cdtext* text = track_get_cdtext(track);

        tmpItem.tag.album = album;
        tmpItem.tag.year = year;

        data = cdtext_get(PTI_TITLE, text);
        if (data != NULL) {
            tmpItem.tag.title = data;
            delete data;
        }

        data = cdtext_get(PTI_PERFORMER, text);
        if (data != NULL) {
            tmpItem.tag.artist = data;
            delete data;
        } else {
            tmpItem.tag.artist = artist;
        }

        data = cdtext_get(PTI_GENRE, text);
        if (data != NULL) {
            tmpItem.tag.genre = data;
            delete data;
        } else {
            tmpItem.tag.genre = genre;
        }

        tmpItem.tag.track = i;

        //cdtext_delete(text);

        cout << i << '\t' << tmpItem.url << endl;
        cout << "range:" << tmpItem.msBeg << "-" << tmpItem.msEnd << endl;
    }
}
