#include "CuePack.h"
#include <mous/MediaItem.h>
#include <scx/FileHelp.hpp>
#include <stdio.h>
using namespace scx;

CuePack::CuePack()
{

}

CuePack::~CuePack()
{

}

void CuePack::GetFileSuffix(std::vector<std::string>& list) const
{
    list.clear();
    list.push_back("cue");
}

#include <iostream>
void CuePack::DumpMedia(const std::string& path, std::deque<MediaItem*>& list,
	const std::map<std::string, IMediaPack*>* pMap) const
{
    FILE* file = fopen(path.c_str(), "r");
    Cd* cd = cue_parse_file(file);
    fclose(file);

    string dir = FileDir(path);

    int ntrack = cd_get_ntrack(cd);
    for (int i = 1; i <= ntrack; ++i) {
	Track* track = cd_get_track(cd, i);
	MediaItem* item = new MediaItem;
	item->url = dir + track_get_filename(track);
	item->hasRange = true;
	item->msBeg = (track_get_start(track))/75*1000;
	item->msEnd = item->msBeg + ((uint64_t)track_get_length(track))/75*1000;
	if (item->msBeg == item->msEnd && i == ntrack)
	    item->msEnd = -1;
	list.push_back(item);

	cout << i << '\t' << item->url << endl;
	cout << "range:" << item->msBeg << "-" << item->msEnd << endl;
    }

}

void CuePack::DumpStream(const std::string& stream, std::deque<MediaItem*>& list,
	const std::map<std::string, IMediaPack*>* pMap) const
{

}
