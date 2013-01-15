#include "cmd.h"

#include <deque>
using namespace std;

#include <scx/Mutex.hpp>
#include <scx/Thread.hpp>
#include <scx/Signal.hpp>
#include <scx/FileInfo.hpp>
using namespace scx;

#include <util/Playlist.h>
#include <core/IPlayer.h>
using namespace mous;

static bool gStop = false;
static Mutex gMutexForSwitch;

static IPlayer* player = NULL;
static Playlist<MediaItem>* playlist = NULL;

void on_finished()
{
    gMutexForSwitch.Lock();
    if (playlist != NULL && !gStop) {
        if (playlist->SeqHasOffset(1)) {
            const MediaItem& item = playlist->SeqItemAtOffset(1, true);
            if (player->Status() != PlayerStatus::Closed)
                player->Close();
            player->Open(item.url);
            if (item.hasRange)
                player->Play(item.msBeg, item.msEnd);
            else
                player->Play();
        }
    }
    gMutexForSwitch.Unlock();
    cout << "Finished!" << endl;
}

void on_playing()
{
    while (true) {
        if (player == NULL || gStop)
            break;
        gMutexForSwitch.Lock();
        uint64_t ms = player->OffsetMs();
        cout << player->BitRate() << " kbps " <<
            ms/1000/60 << ":" << ms/1000%60 << "." << ms%1000 << '\r' << flush;
        gMutexForSwitch.Unlock();
        usleep(200*1000);
    }
}

int cmd_play(int argc, char* argv[])
{
    int ret = 0;


    // setup player
    player = IPlayer::Create();
    player->SigFinished()->Connect(&on_finished);
    player->RegisterRendererPlugin(ctx.red_agents[0]);
    player->RegisterDecoderPlugin(ctx.dec_agents);

    // setup playlist
    playlist = new Playlist<MediaItem>();
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
    playlist->SetMode(PlaylistMode::Repeat);

    if (playlist->Empty()) {
        ret = -1;
        goto LABEL_END;
    }

    // Begin to play.
    {
        const MediaItem& item = playlist->SeqItemAtOffset(0, false);

        cout << ">> Tag Info" << endl;
        cout << "\ttitle(" << item.tag.title.size() << "):" << item.tag.title << endl;
        cout << "\tartist(" << item.tag.artist.size() << "):" << item.tag.artist << endl;
        cout << "\talbum(" << item.tag.album.size() << "):" << item.tag.album << endl;
        cout << "\tcomment:" << item.tag.comment << endl;
        cout << "\tgenre:" << item.tag.genre << endl;
        cout << "\tyear:" << item.tag.year << endl;
        cout << "\ttrack:" << item.tag.track << endl;

        cout << "item.url:" << item.url << endl;
        player->Open(item.url);
        if (item.hasRange) {
            player->Play(item.msBeg, item.msEnd);
        } else {
            player->Play();
        }
        Thread thread;
        thread.Run(Function<void (void)>(&on_playing));

        bool paused;
        char ch = ' ';
        while (ch != 'q') {
            cin >> ch;
            switch (ch) {
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
                if (item.hasRange) {
                    player->Play(item.msBeg, item.msEnd);
                } else {
                    player->Play();
                }
                break;
            }
        }

        gStop = true;
        thread.Join();
    }

    //int ch;
    //while ( ( ch = getopt(argc, argv, "") ) != -1 ) {
    //}

LABEL_END:
    playlist->Clear();
    delete playlist;

    player->UnregisterAll();
    IPlayer::Free(player);

    return ret;
}
