#include "cmd.h"

#include <iostream>
#include <iomanip>
#include <sstream>
#include <deque>
using namespace std;

#include <scx/FileInfo.hpp>
using namespace scx;

using namespace mous;

static string ms2str(size_t ms)
{
    int min = ms/1000/60;
    int sec = ms/1000%60;
    stringstream stream;
    stream << setfill('0') << setw(2) << min << ":" << setfill('0') << setw(2) << sec;
    return stream.str();
}

int cmd_info(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
        FileInfo info(argv[i]);
        if (!info.Exists() || (info.Type() == FileType::Directory)) {
            cout << "invaild file: " << argv[i] << endl;
            continue;
        }

        cout.precision(2);

        // phy info
        {
            size_t size = info.Size();
            double dsize = size;
            char csize = 'B';
            if (size/1024/1024 > 0) {
                csize = 'M';
                dsize = size/1024/1024.f;
            } else if (size/1024 > 0) {
                csize = 'K';
                dsize = size/1024.f;
            }
            cout << "file:      " << info.BaseName() << endl;
            cout << "path:      " << info.AbsPath() << endl;
            cout << "size:      " << dsize << csize << " (" << size << "B)" << endl;
        }

        // tag info
        {
            deque<MediaItem> media_list;
            ctx.loader->LoadMedia(argv[i], media_list);
            for (auto& item: media_list) {
                // duration
                if (item.hasRange) {
                    if (item.msEnd == (uint64_t)-1)
                        item.msEnd = item.duration;
                    cout << "range:     " << ms2str(item.msBeg) << " - " << ms2str(item.msEnd) << endl;
                    cout << "duration:  " << ms2str(item.msEnd - item.msBeg) << endl;
                } else {
                    cout << "duration:  " << ms2str(item.duration) << endl;
                }
                // tag
                cout << "title:     " << item.tag.title << endl;
                cout << "artist:    " << item.tag.artist << endl;
                cout << "album:     " << item.tag.album << endl;
                cout << "comment:   " << item.tag.comment << endl;
                cout << "genre:     " << item.tag.genre << endl;
                cout << "year:      " << item.tag.year << endl;
                cout << "track:     " << item.tag.track << endl;
            }
        }

        cout << endl;
    }

    return 0;
}
