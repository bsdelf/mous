#include "cmd.h"

#include <unistd.h>

#include <deque>
using namespace std;

#include <scx/Mutex.hpp>
#include <scx/Thread.hpp>
#include <scx/Signal.hpp>
#include <scx/FileInfo.hpp>
using namespace scx;

#include "util/Playlist.h"
#include "core/IPlayer.h"
using namespace mous;

static bool stop = false;
static Mutex player_mutex;

static IPlayer* player = NULL;
static Playlist<MediaItem>* playlist = NULL;

void on_finished()
{
    if (playlist != NULL && !stop) {
        if (playlist->SeqHasOffset(1)) {
            const MediaItem& item = playlist->SeqItemAtOffset(1, true);

            MutexLocker locker(&player_mutex);
            if (player->Status() != PlayerStatus::Closed)
                player->Close();
            player->Open(item.url);
            if (item.hasRange)
                player->Play(item.msBeg, item.msEnd);
            else
                player->Play();
        }
    }
    cout << "finished!" << endl;
}

void do_playing()
{
    while (true) {
        if (player == NULL || stop)
            break;

        player_mutex.Lock();
        uint64_t ms = player->OffsetMs();
        int32_t rate = player->BitRate();
        player_mutex.Unlock();

        cout << rate << " kbps "
             << ms/1000/60 << ":" << ms/1000%60 << "." << ms%1000
             << '\r' << flush;

        usleep(200*1000);
    }
}

int cmd_play(int argc, char* argv[])
{
    int rval = 0;

    // init player
    player = IPlayer::Create();
    player->SigFinished()->Connect(&on_finished);
    player->RegisterRendererPlugin(ctx.red_agents[0]);
    player->RegisterDecoderPlugin(ctx.dec_agents);
    playlist = new Playlist<MediaItem>();
    playlist->SetMode(PlaylistMode::Normal);

    // parse arguments
    for (int ch = -1; (ch = getopt(argc, argv, "rs")) != -1; ) {
        switch (ch) {
        case 'r':
            playlist->SetMode(playlist->Mode() != PlaylistMode::Shuffle ?
                              PlaylistMode::Repeat :
                              PlaylistMode::ShuffleRepeat);
            break;
            
        case 's':
            playlist->SetMode(playlist->Mode() != PlaylistMode::Repeat ?
                              PlaylistMode::Shuffle :
                              PlaylistMode::ShuffleRepeat);
            break;

        default:
            goto LABEL_CLEANUP;
        }
    }
    argc -= optind;
    argv += optind;

    // build playlist
    for (int i = 0; i < argc; ++i) {
        deque<MediaItem> media_list;
        FileInfo info(argv[i]);
        if (info.Exists() && (info.Type() != FileType::Directory)) {
            ctx.loader->LoadMedia(argv[i], media_list);
            playlist->Append(media_list);
        } else {
            cout << "invaild file: " << argv[i] << endl;
        }
    }
    if (playlist->Empty()) {
        cout << "playlist is empty!" << endl;
        rval = -1;
        goto LABEL_CLEANUP;
    }

    // begin to play
    {
        cout << "[n(next)/q(quit)/p(pause)/r(replay)] [enter]" << endl;

        const MediaItem& item = playlist->SeqItemAtOffset(0, false);

        cout << "playing: \"" << item.url << "\""<< endl;
        player->Open(item.url);
        if (item.hasRange) {
            player->Play(item.msBeg, item.msEnd);
        } else {
            player->Play();
        }
        Thread thread;
        thread.Run(Function<void (void)>(&do_playing));

        bool paused;
        for (char ch = ' '; ch != 'q'; ) {
            cin >> ch;

            MutexLocker locker(&player_mutex);
            switch (ch) {
            case 'n':
                on_finished();
                break;

            case 'q':
                player->Close();
                break;

            case 'p':
                if (paused) {
                    player->Resume();
                    paused = false;
                } else {
                    player->Pause();
                    paused = true;
                }
                break;

            case 'r':
                player->Pause();
                if (item.hasRange) {
                    player->Play(item.msBeg, item.msEnd);
                } else {
                    player->Play();
                }
                break;
            }
        }

        stop = true;
        thread.Join();
    }

    // cleanup
LABEL_CLEANUP:
    playlist->Clear();
    delete playlist;
    player->UnregisterAll();
    IPlayer::Free(player);

    return rval;
}
