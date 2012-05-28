#include <util/MediaItem.h>
#include <util/PlaylistSerializer.h>
using namespace mous;

#include "scx/BufObj.hpp"
#include "scx/Conv.hpp"
using namespace scx;

#include <iostream>
#include <fstream>
using namespace std;

const char* const FILE_NAME = "list.dat";
const int COUNT = 10;

void PrintList(const Playlist<MediaItem>& list)
{
    for (int i = 0; i < list.Count(); ++i) {
        const MediaItem& item = list[i];
        cout << i << "|" << item.url << "|" << item.duration << "|";
        cout << item.tag.title << "|" << item.tag.artist << "|" << item.tag.album << "|" << item.tag.year << endl;
    }
}

int main()
{
    PlaylistSerializer<MediaItem> serializer;

    cout << "=== Write dat ===" << endl;
    {
        Playlist<MediaItem> list;

        for (int i = 0; i < COUNT; ++i) {
            MediaItem item;
            item.url = "url" + NumToStr(i);
            item.duration = 100*i;
            item.hasRange = false;
            item.msBeg = 0;
            item.msEnd = 0;
            item.tag.title = "title" + NumToStr(i);
            item.tag.artist = "artist" + NumToStr(i);
            item.tag.album = "album" + NumToStr(i);
            item.tag.comment = "comment" + NumToStr(i);
            item.tag.genre = "genre" + NumToStr(i);
            item.tag.year = 1990+i;
            item.tag.track = i;
            list.Append(item);
        }

        PrintList(list);

        serializer.Store(list, FILE_NAME);

        vector<char> outbuf;
        serializer.Store(list, outbuf);
        cout << COUNT << " items, write bytes:" << outbuf.size() << endl;
    }

    cout << "=== Read dat ===" << endl;
    {
        Playlist<MediaItem> list2;
        serializer.Load(list2, FILE_NAME);
        PrintList(list2);
    }

    return 0;
}
