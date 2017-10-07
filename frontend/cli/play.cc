#include <unistd.h>
#include <stdio.h>

#include <thread>
#include <mutex>
#include <deque>

#include <scx/Signal.h>
#include <scx/FileInfo.h>
using namespace scx;

#include <util/Playlist.h>
#include <core/Player.h>
using namespace mous;

#include "ctx.h"
#include "cmd.h"

static bool QUIT = false;
static std::mutex PLAYER_MUTEX;

static Player PLAYER;
static Playlist<MediaItem>* PLAYLIST = nullptr;

void on_finished()
{
    if (PLAYLIST == nullptr)
        return;

    if (PLAYLIST->HasNext(1)) {
        const MediaItem& item = PLAYLIST->NextItem(1, true);

        printf("playing: \"%s\"\n", item.url.c_str());

        std::lock_guard<std::mutex> locker(PLAYER_MUTEX);
        if (PLAYER.Status() != PlayerStatus::Closed)
            PLAYER.Close();
        PLAYER.Open(item.url);
        if (item.hasRange)
            PLAYER.Play(item.msBeg, item.msEnd);
        else
            PLAYER.Play();
    } else {
        QUIT = true;
    }
    //printf("finished!\n");
}

int cmd_play(int argc, char* argv[])
{
    int rval = 0;

    // init player
    PLAYER.SigFinished()->Connect(&on_finished);
    PLAYER.RegisterRendererPlugin(ctx.outputPlugins[0]);
    PLAYER.RegisterDecoderPlugin(ctx.decoderPlugins);
    PLAYLIST = new Playlist<MediaItem>();
    PLAYLIST->SetMode(PlaylistMode::Normal);

    // parse arguments
    for (int ch = -1; (ch = getopt(argc, argv, "rs")) != -1; ) {
        switch (ch) {
        case 'r':
            PLAYLIST->SetMode(PLAYLIST->Mode() != PlaylistMode::Shuffle ?
                              PlaylistMode::Repeat :
                              PlaylistMode::ShuffleRepeat);
            break;
            
        case 's':
            PLAYLIST->SetMode(PLAYLIST->Mode() != PlaylistMode::Repeat ?
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
            ctx.loader.LoadMedia(argv[i], media_list);
            PLAYLIST->Append(media_list);
        } else {
            printf("invaild file: %s\n", argv[i]);
        }
    }
    if (PLAYLIST->Empty()) {
        printf("playlist is empty!\n");
        rval = -1;
        goto LABEL_CLEANUP;
    }

    // begin to play
    {
        on_finished();
        auto th = std::thread([] {
            while (!QUIT) {
                PLAYER_MUTEX.lock();
                uint64_t ms = PLAYER.OffsetMs();
                int32_t rate = PLAYER.BitRate();
                PLAYER_MUTEX.unlock();

                printf("\r%d kbps %02d:%02d.%d ",
                        rate, (int)(ms/1000/60), (int)(ms/1000%60), (int)(ms%1000));
                fflush(stdout);

                usleep(200*1000);
            }
        });

        if (th.joinable())
            th.join();
    }

    // cleanup
LABEL_CLEANUP:
    PLAYLIST->Clear();
    delete PLAYLIST;
    PLAYER.UnregisterAll();

    return rval;
}
