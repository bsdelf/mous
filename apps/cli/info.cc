#include <stdio.h>
#include <iomanip>
#include <sstream>
#include <deque>

#include <scx/FileInfo.h>
using namespace scx;

#include "ctx.h"
#include "cmd.h"

static std::string ms2str(size_t ms)
{
    int min = ms/1000/60;
    int sec = ms/1000%60;
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2) << min << ":" << std::setfill('0') << std::setw(2) << sec;
    return stream.str();
}

int cmd_info(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
        FileInfo info(argv[i]);
        if (!info.Exists() || (info.Type() == FileType::Directory)) {
            printf("invalid file: %s\n", argv[i]);
            continue;
        }

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
            printf("file: %s\n", info.BaseName().c_str());
            printf("path: %s\n", info.AbsPath().c_str());
            printf("size: %lf%c (%zuB)\n", dsize, csize, size);
        }

        // tag info
        {
            std::deque<MediaItem> media_list;
            ctx->mediaLoader.LoadMedia(argv[i], media_list);
            for (auto& item: media_list) {
                // duration
                if (item.hasRange) {
                    if (item.msEnd == (uint64_t)-1)
                        item.msEnd = item.duration;
                    printf("range:      %s - %s\n", ms2str(item.msBeg).c_str(), ms2str(item.msEnd).c_str());
                    printf("duration:   %s\n", ms2str(item.msEnd - item.msBeg).c_str());
                } else {
                    printf("duration:   %s\n", ms2str(item.duration).c_str());
                }
                // tag
                printf("title:    %s\n", item.tag.title.c_str());
                printf("artist:   %s\n", item.tag.artist.c_str());
                printf("album:    %s\n", item.tag.album.c_str());
                printf("comment:  %s\n", item.tag.comment.c_str());
                printf("genre:    %s\n", item.tag.genre.c_str());
                printf("year:     %d\n", item.tag.year);
                printf("track:    %d\n", item.tag.track);
            }
        }

        printf("\n");
    }

    return 0;
}
