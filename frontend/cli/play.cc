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

static bool QUIT = false;
static Mutex PLAYER_MUTEX;

static IPlayer* PLAYER = NULL;
static Playlist<MediaItem>* PLAYLIST = NULL;

void on_finished()
{
    if (PLAYLIST == NULL)
        return;

    if (PLAYLIST->SeqHasOffset(1)) {
        const MediaItem& item = PLAYLIST->SeqItemAtOffset(1, true);

        cout << "playing: \"" << item.url << "\""<< endl;

        MutexLocker locker(&PLAYER_MUTEX);
        if (PLAYER->Status() != PlayerStatus::Closed)
            PLAYER->Close();
        PLAYER->Open(item.url);
        if (item.hasRange)
            PLAYER->Play(item.msBeg, item.msEnd);
        else
            PLAYER->Play();
    } else {
        QUIT = true;
    }
    //cout << "finished!" << endl;
}

void do_playing()
{
    while (PLAYER != NULL && !QUIT) {
        PLAYER_MUTEX.Lock();
        uint64_t ms = PLAYER->OffsetMs();
        int32_t rate = PLAYER->BitRate();
        PLAYER_MUTEX.Unlock();

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
    PLAYER = IPlayer::Create();
    PLAYER->SigFinished()->Connect(&on_finished);
    PLAYER->RegisterRendererPlugin(ctx.red_agents[0]);
    PLAYER->RegisterDecoderPlugin(ctx.dec_agents);
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
            PLAYLIST->Append(media_list);
        } else {
            cout << "invaild file: " << argv[i] << endl;
        }
    }
    if (PLAYLIST->Empty()) {
        cout << "PLAYLIST is empty!" << endl;
        rval = -1;
        goto LABEL_CLEANUP;
    }

    // begin to play
    {
        //cout << "[n(next)/q(quit)/p(pause)/r(replay)] [enter]" << endl;

        on_finished();
        Thread thread;
        thread.Run(Function<void (void)>(&do_playing));

        /*
        bool paused;
        for (char ch = ' '; ch != 'q'; ) {
            cin >> ch;

            MutexLocker locker(&PLAYER_MUTEX);
            switch (ch) {
            case 'n':
                on_finished();
                break;

            case 'q':
                PLAYER->Close();
                break;

            case 'p':
                if (paused) {
                    PLAYER->Resume();
                    paused = false;
                } else {
                    PLAYER->Pause();
                    paused = true;
                }
                break;

            case 'r':
                PLAYER->Pause();
                if (item.hasRange) {
                    PLAYER->Play(item.msBeg, item.msEnd);
                } else {
                    PLAYER->Play();
                }
                break;
            }
        }
        */

        thread.Join();
    }

    // cleanup
LABEL_CLEANUP:
    PLAYLIST->Clear();
    delete PLAYLIST;
    PLAYER->UnregisterAll();
    IPlayer::Free(PLAYER);

    return rval;
}
