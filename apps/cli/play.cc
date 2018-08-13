#include <unistd.h>
#include <stdio.h>

#include <thread>
#include <mutex>
#include <deque>

#include <scx/Signal.h>
#include <scx/FileInfo.h>
using namespace scx;

#include "ctx.h"
#include "cmd.h"

void on_finished()
{
    if (ctx->playlist.HasNext(1)) {
        const MediaItem& item = ctx->playlist.NextItem(1, true);

        printf("playing: \"%s\"\n", item.url.c_str());

        std::lock_guard<std::mutex> locker(ctx->playerMutex);
        if (ctx->player.Status() != PlayerStatus::Closed) {
            ctx->player.Close();
        }
        ctx->player.Open(item.url);
        if (item.hasRange) {
            ctx->player.Play(item.msBeg, item.msEnd);
        } else {
            ctx->player.Play();
        }
    } else {
        ctx->playerQuit = true;
    }
    //printf("finished!\n");
}

int cmd_play(int argc, char* argv[])
{
    int rval = 0;

    // init player
    ctx->player.SigFinished()->Connect(&on_finished);
    ctx->playlist.SetMode(PlaylistMode::Normal);

    // parse arguments
    for (int ch = -1; (ch = getopt(argc, argv, "rs")) != -1; ) {
        switch (ch) {
        case 'r':
            ctx->playlist.SetMode(ctx->playlist.Mode() != PlaylistMode::Shuffle ?
                              PlaylistMode::Repeat :
                              PlaylistMode::ShuffleRepeat);
            break;
            
        case 's':
            ctx->playlist.SetMode(ctx->playlist.Mode() != PlaylistMode::Repeat ?
                              PlaylistMode::Shuffle :
                              PlaylistMode::ShuffleRepeat);
            break;

        default:
            rval = 1;
            goto LABEL_CLEANUP;
        }
    }
    argc -= optind;
    argv += optind;

    // build playlist
    for (int i = 0; i < argc; ++i) {
        std::deque<MediaItem> media_list;
        FileInfo info(argv[i]);
        if (info.Exists() && (info.Type() != FileType::Directory)) {
            ctx->mediaLoader.LoadMedia(argv[i], media_list);
            ctx->playlist.Append(media_list);
        } else {
            printf("invaild file: %s\n", argv[i]);
        }
    }
    if (ctx->playlist.Empty()) {
        printf("playlist is empty!\n");
        rval = -1;
        goto LABEL_CLEANUP;
    }

    // begin to play
    {
        on_finished();
        auto th = std::thread([] {
            while (!ctx->playerQuit) {
                ctx->playerMutex.lock();
                uint64_t ms = ctx->player.OffsetMs();
                int32_t rate = ctx->player.BitRate();
                ctx->playerMutex.unlock();
                printf("\r%d kbps %02d:%02d.%d ",
                    rate, (int)(ms/1000/60), (int)(ms/1000%60), (int)(ms%1000));
                fflush(stdout);
                usleep(200*1000);
            }
        });
        if (th.joinable()) {
            th.join();
        }
    }

    // cleanup
LABEL_CLEANUP:
    ctx->playlist.Clear();

    return rval;
}
