#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <util/MediaItem.h>
#include <util/PluginOption.h>
#include <util/Playlist.h>
#include <core/IPlayer.h>
#include <core/IMediaLoader.h>
#include <core/IPluginManager.h>
#include <core/IConvTask.h>
#include <core/IConvTaskFactory.h>
#include <scx/Mutex.hpp>
#include <scx/Thread.hpp>
#include <scx/Signal.hpp>
using namespace std;
using namespace scx;
using namespace mous;

bool gStop = false;
IPlayer* gPlayer = NULL;
Playlist<MediaItem*>* gPlaylist = NULL;
Mutex gMutexForSwitch;

void OnFinished()
{
    gMutexForSwitch.Lock();
    if (gPlaylist != NULL && !gStop) {
        MediaItem* item = NULL;
        if (gPlaylist->SeqCurrent(item, 1)) {
            gPlaylist->SeqMoveNext();
            gPlaylist->SeqCurrent(item);
            if (gPlayer->GetStatus() != PlayerStatus::Closed)
                gPlayer->Close();
            gPlayer->Open(item->url);
            if (item->hasRange)
                gPlayer->Play(item->msBeg, item->msEnd);
            else
                gPlayer->Play();
        }
    }
    gMutexForSwitch.Unlock();
    cout << "Finished!" << endl;
}

void OnPlaying()
{
    while (true) {
        if (gPlayer == NULL || gStop)
            break;
        gMutexForSwitch.Lock();
        uint64_t ms = gPlayer->GetOffsetMs();
        cout << gPlayer->GetBitRate() << " kbps " <<
            ms/1000/60 << ":" << ms/1000%60 << "." << ms%1000 << '\r' << flush;
        gMutexForSwitch.Unlock();
        usleep(200*1000);
    }
}

/*
#include <CharsetConv/CharsetConv.h>
//#include <enca.h>
*/

void PrintPluginOption(vector<PluginOption>& list)
{
    for (size_t i = 0; i < list.size(); ++i) {
        PluginOption& opt = list[i];
        cout << ">>>> index:" << i+1 << endl;
        cout << "\tplugin type: " << ToString(opt.pluginType)<< endl;
        for (size_t io = 0; io < opt.options.size(); ++io) {
            cout << "\t\t option type: " << ToString(opt.options[io]->type) << endl;
            cout << "\t\t option desc: " << opt.options[io]->desc << endl;
        }
    }
}

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
    mgr->GetPlugins(decoderAgentList, PluginType::Decoder);
    cout << ">> Decoder count:" << decoderAgentList.size() << endl;

    vector<const IPluginAgent*> encoderAgentList;
    mgr->GetPlugins(encoderAgentList, PluginType::Encoder);
    cout << ">> Encoder count:" << encoderAgentList.size() << endl;

    vector<const IPluginAgent*> rendererAgentList;
    mgr->GetPlugins(rendererAgentList, PluginType::Renderer);
    cout << ">> Renderer count:" << rendererAgentList.size() << endl;

    vector<const IPluginAgent*> packAgentList;
    mgr->GetPlugins(packAgentList, PluginType::MediaPack);
    cout << ">> MediaPack count:" << packAgentList.size() << endl;

    vector<const IPluginAgent*> tagAgentList;
    mgr->GetPlugins(tagAgentList, PluginType::TagParser);
    cout << ">> TagParser count:" << tagAgentList.size() << endl;
    cout << endl;

    vector<const IPluginAgent*> pelAgentList;
    mgr->GetPlugins(pelAgentList, PluginType::EventWatcher);
    cout << ">> EventWatcher count:" << pelAgentList.size() << endl;

    // Check plugins enough.
    if (decoderAgentList.empty() || rendererAgentList.empty())
        return -2;

    // Setup loader
    IMediaLoader* loader = IMediaLoader::Create();
    for (size_t i = 0; i < packAgentList.size(); ++i) {
        loader->RegisterMediaPackPlugin(packAgentList[i]);
    }
    for (size_t i = 0; i < tagAgentList.size(); ++i) {
        loader->RegisterTagParserPlugin(tagAgentList[i]);
    }

    // Setup playlist
    Playlist<MediaItem*> playlist;
    gPlaylist = &playlist;
    deque<MediaItem*> mediaList;
    for (int i = 1; i < argc; ++i) {
        loader->LoadMedia(argv[i], mediaList);
        playlist.AppendItem(mediaList);
    }
    playlist.SetMode(PlaylistMode::Repeat);

    // test for encoder
    if (false)
    {
        IConvTaskFactory* factory = IConvTaskFactory::Create();
        for (size_t i = 0; i < encoderAgentList.size(); ++i) {
            factory->RegisterDecoderPlugin(encoderAgentList[i]);
        }
        for (size_t i = 0; i < decoderAgentList.size(); ++i) {
            factory->RegisterEncoderPlugin(decoderAgentList[i]);
        }

        vector<string> encoders = factory->GetEncoderNames();
        if (encoders.empty()) {
            cout << "No encoders!" << endl;
            IConvTaskFactory::Free(factory);
            return 0;
        }

        cout << ">> Available encoders:" << endl;
        for (size_t i = 0; i < encoders.size(); ++i) {
            cout << i+1 << ": " << encoders[i] << endl;
        }
        int index;
        cout << ">> Select encoder:";
        cin >> index;
        if (index < 1) {
            IConvTaskFactory::Free(factory);
            return 0;
        }

        MediaItem* item = playlist.GetItem(11);
        cout << item->url << endl;
        IConvTask* task = factory->CreateTask(item, encoders[index-1]);
        task->Run("output.wav");

        while (!task->IsFinished()) {
            double percent =  task->GetProgress();
            if (percent < 0) {
                cout << "failed!" << endl;
                break;
            }
            cout << "progress:" << (int)(percent*100) << "%" << "\r" << flush;
            usleep(1000);
        }
        cout << "quit!" << endl;

        IConvTask::Free(task);
        IConvTaskFactory::Free(factory);
        return 0;
    }

    // Setup player
    IPlayer* player = IPlayer::Create();
    player->SigFinished()->Connect(&OnFinished);
    player->RegisterRendererPlugin(rendererAgentList[0]);
    for (size_t i = 0; i < decoderAgentList.size(); ++i) {
        player->RegisterDecoderPlugin(decoderAgentList[i]);
    }
    for (size_t i = 0; i < pelAgentList.size(); ++i) {
        //player->RegisterPlugin(pelAgentList[i]);
    }

    // Show player options 
    {
        vector<PluginOption> list;
        cout << ">> Player decoder plugin options:" << endl;
        player->GetDecoderPluginOption(list);
        PrintPluginOption(list);
        cout << ">> Player renderer plugin options:" << endl;
        PluginOption opt;
        player->GetRendererPluginOption(opt);
        list.resize(1);
        list[0] = opt;
        PrintPluginOption(list);
    }

    // Begin to play.
    if (playlist.Empty())
        return -1;

    MediaItem* item = NULL;
    playlist.SeqCurrent(item);
    assert(item != NULL);
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
    mgr->UnloadAll();
    
    for (size_t i = 0; i < mediaList.size(); ++i) {
        delete mediaList[i];
    }

    IPlayer::Free(player);
    IMediaLoader::Free(loader);
    IPluginManager::Free(mgr);

    return 0;
}
