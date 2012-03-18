#include <iostream>
#include <vector>
#include <string>
#include <common/MediaItem.h>
#include <core/IPlayer.h>
#include <core/IPlaylist.h>
#include <core/IMediaLoader.h>
#include <core/IPluginManager.h>
#include <scx/Thread.hpp>
#include <scx/AsyncSignal.hpp>
#include <scx/FileHelp.hpp>
using namespace std;
using namespace scx;
using namespace mous;

bool gStop = false;
IPlayer* gPlayer = NULL;

void OnFinished()
{
	cout << "Finished!" << endl;
}

void OnPlaying()
{
	while (true) {
		if (gPlayer == NULL || gStop)
			break;
		uint64_t ms = gPlayer->GetCurrentMs();
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
		cout << ">>>> " << info->description << endl;
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

	// Check args enough.
	if (argc < 2) {
		cout << "Usage:" << endl;
		cout << "mous-cli [-r] [file]" << endl;
		cout << "-r\tRepeat mode." << endl;
		return -1;
	}

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

	IPlaylist* playlist = IPlaylist::Create();

	deque<MediaItem*> mediaList;
	loader->LoadMedia(argv[1], mediaList);

	// Setup player->
	IPlayer* player = IPlayer::Create();
	player->SetRendererDevice("/dev/dsp");
	player->SigFinished()->Connect(&OnFinished);
	player->RegisterPluginAgent(rendererAgentList[0]);
	for (size_t i = 0; i < decoderAgentList.size(); ++i) {
		player->RegisterPluginAgent(decoderAgentList[i]);
	}

	// Begin to play.
	MediaItem* item = mediaList[0];
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

    IPlayer::Free(player);
    IPluginManager::Free(mgr);
    IMediaLoader::Free(loader);
    IPlaylist::Free(playlist);

	return 0;
}
