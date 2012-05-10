#include "util/Playlist.h"
#include "util/MediaItem.h"
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

        BufObj buf(NULL);
        list >> buf;
        cout << COUNT << " items, need bytes:" << buf.GetOffset() << endl;

        vector<char> realBuf(buf.GetOffset());
        buf.ResetOffset();
        buf.SetBuffer(&realBuf[0]);
        list >> buf;

        fstream file;
        file.open(FILE_NAME, ios::out);
        file << (int)realBuf.size();
        file.write(&realBuf[0], realBuf.size());
        file.close();
    }

    cout << "=== Read dat ===" << endl;
    {
        vector<char> realBuf;
        int size;

        fstream file;
        file.open(FILE_NAME, ios::in);
        file >> size;
        realBuf.resize(size);
        file.read(&realBuf[0], size);
        file.close();

        cout << "bytes:" << size << endl;

        BufObj buf(&realBuf[0]);
        Playlist<MediaItem> list2;
        list2 << buf;
        PrintList(list2);
    }

    return 0;
}
