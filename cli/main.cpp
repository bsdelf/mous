#include <iostream>
#include <vector>
#include <string>
#include <common/MediaItem.h>
#include <common/PluginOption.h>
#include <core/IPlayer.h>
#include <core/IPlaylist.h>
#include <core/IMediaLoader.h>
#include <core/IPluginManager.h>
#include <scx/Thread.hpp>
#include <scx/AsyncSignal.hpp>
using namespace std;
using namespace scx;
using namespace mous;

bool gStop = false;
IPlayer* gPlayer = NULL;
IPlaylist* gPlaylist = NULL;

void OnFinished()
{
    if (gPlaylist != NULL && !gStop) {
        MediaItem* item = (MediaItem*)gPlaylist->SeqCurrent(1);
        if (item != NULL) {
            gPlaylist->SeqMoveNext();
            item = (MediaItem*)gPlaylist->SeqCurrent();
            if (gPlayer->GetStatus() != PlayerStatus::Closed)
                gPlayer->Close();
            gPlayer->Open(item->url);
            if (item->hasRange)
                gPlayer->Play(item->msBeg, item->msEnd);
            else
                gPlayer->Play();
        }
    }
    cout << "Finished!" << endl;
}

void OnPlaying()
{
    while (true) {
        if (gPlayer == NULL || gStop)
            break;
        uint64_t ms = gPlayer->GetOffsetMs();
        cout << gPlayer->GetBitRate() << " kbps " <<
            ms/1000/60 << ":" << ms/1000%60 << "." << ms%1000 << '\r' << flush;
        usleep(200*1000);
    }
}

/*
#include <CharsetConv/CharsetConv.h>
//#include <enca.h>
*/

int main(int argc, char** argv)
{
    // Check args enough.
    if (argc < 2) {
        cout << "Usage:" << endl;
        cout << argv[0] << " [file1] [file2] [...]" << endl;
        return -1;
    }

    bool paused = false;

    /*
    string content(ReadAll(argv[1]));
    cout << "len:" << content.length() << endl;

    UErrorCode uerr = U_ZERO_ERROR;
    int32_t found = 1;
    UCharsetDetector* udec = ucsdet_open(&uerr);
    ucsdet_setText(udec, content.c_str(), content.length(), &uerr);
    const UCharsetMatch** match = ucsdet_detectAll(udec, &found, &uerr);
    for (int i = 0; i < found; ++i) {
        cout << ucsdet_getName(match[i], &uerr) << '\t';
        cout << ucsdet_getConfidence(match[i], &uerr) << endl;
    }
    cout << found << endl;
    ucsdet_close(udec);
    return 0;
    */

    /*
    CharsetConv conv;
    string output;
    bool ok = conv.AutoConv(content.c_str(), content.length(), output);
    cout << (ok ? output : content) << endl;
    return 0;
    */

    //EncaAnalyser ans = enca_analyser_alloc("uk");
    //EncaEncoding enc = enca_analyse_const(ans, (const unsigned char*)content.c_str(), content.length());
    //cout << enca_charset_name(enc.charset, ENCA_NAME_STYLE_ICONV) << endl;

    IPluginManager* mgr = IPluginManager::Create();
    mgr->LoadPluginDir("./plugins");

    // Dump all plugin path.
    vector<string> pathList;
    mgr->GetPluginPath(pathList);
    for (size_t i = 0; i < pathList.size(); ++i) {
        cout << ">> " << pathList[i] << endl;
        const PluginInfo* info = mgr->GetPluginInfo(pathList[i]);
        cout << ">>>> " << info->author << endl;
        cout << ">>>> " << info->name << endl;
        cout << ">>>> " << info->desc << endl;
        cout << ">>>> " << info->version << endl;
    }
    cout << endl;

    // Get all plugin agents.
    vector<const IPluginAgent*> decoderAgentList;
    mgr->GetPluginAgents(decoderAgentList, PluginType::Decoder);
    cout << ">> Decoder count:" << decoderAgentList.size() << endl;

    vector<const IPluginAgent*> rendererAgentList;
    mgr->GetPluginAgents(rendererAgentList, PluginType::Renderer);
    cout << ">> Renderer count:" << rendererAgentList.size() << endl;

    vector<const IPluginAgent*> packAgentList;
    mgr->GetPluginAgents(packAgentList, PluginType::MediaPack);
    cout << ">> MediaPack count:" << packAgentList.size() << endl;

    vector<const IPluginAgent*> tagAgentList;
    mgr->GetPluginAgents(tagAgentList, PluginType::TagParser);
    cout << ">> TagParser count:" << tagAgentList.size() << endl;
    cout << endl;

    vector<const IPluginAgent*> pelAgentList;
    mgr->GetPluginAgents(pelAgentList, PluginType::EventWatcher);
    cout << ">> EventWatcher count:" << pelAgentList.size() << endl;

    // Check plugins enough.
    if (decoderAgentList.empty() || rendererAgentList.empty())
        return -2;

    // Setup loader
    IMediaLoader* loader = IMediaLoader::Create();
    for (size_t i = 0; i < packAgentList.size(); ++i) {
        loader->RegisterPluginAgent(packAgentList[i]);
    }
    for (size_t i = 0; i < tagAgentList.size(); ++i) {
        loader->RegisterPluginAgent(tagAgentList[i]);
    }

    // Setup playlist
    IPlaylist* playlist = IPlaylist::Create();
    playlist->SetPlayMode(PlayMode::ShuffleRepeat);
    gPlaylist = playlist;

    deque<MediaItem*> mediaList;
    for (int i = 1; i < argc; ++i) {
        loader->LoadMedia(argv[i], mediaList);
        playlist->AppendItem(mediaList);
    }

    // Setup player
    IPlayer* player = IPlayer::Create();
    player->SigFinished()->Connect(&OnFinished);
    player->RegisterPluginAgent(rendererAgentList[0]);
    for (size_t i = 0; i < decoderAgentList.size(); ++i) {
        player->RegisterPluginAgent(decoderAgentList[i]);
    }
    for (size_t i = 0; i < pelAgentList.size(); ++i) {
        player->RegisterPluginAgent(pelAgentList[i]);
    }

    // Show player options 
    {
        vector<PluginOption> list;
        player->GetPluginOption(list);
        cout << ">> Plugin options:" << endl;
        for (size_t i = 0; i < list.size(); ++i) {
            cout << ">>>> index:" << i+1 << endl;
            cout << "\tplugin type: " << ToString(list[i].pluginType)<< endl;
            for (size_t i = 0; i < list[i].options.size(); ++i) {
                cout << "\t\t option type: " << ToString(list[i].options[i].second) << endl;
            }
        }
    }

    // Begin to play.
    if (playlist->Empty())
        return -1;

    MediaItem* item = (MediaItem*)playlist->SeqCurrent();
    cout << ">>>> Tag Info" << endl;
    cout << "\ttitle:" << item->title << endl;
    cout << "\tartist:" << item->artist << endl;
    cout << "\talbum:" << item->album << endl;
    cout << "\tcomment:" << item->comment << endl;
    cout << "\tgenre:" << item->genre << endl;
    cout << "\tyear:" << item->year << endl;
    cout << "\ttrack:" << item->track << endl;

    cout << "item->url:" << item->url << endl;
    player->Open(item->url);
    if (item->hasRange) {
        player->Play(item->msBeg, item->msEnd);
    } else {
        player->Play();
    }
    Thread th;
    gPlayer = player;
    th.Run(Function<void (void)>(&OnPlaying));

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
                if (item->hasRange) {
                    player->Play(item->msBeg, item->msEnd);
                } else {
                    player->Play();
                }
                break;
        }
    }

    gStop = true;
    th.Join();

    loader->UnregisterAll();
    player->UnregisterAll();
    mgr->UnloadAllPlugins();
    
    for (size_t i = 0; i < mediaList.size(); ++i) {
        delete mediaList[i];
    }

    IPlayer::Free(player);
    IMediaLoader::Free(loader);
    IPluginManager::Free(mgr);
    IPlaylist::Free(playlist);

    return 0;
}
