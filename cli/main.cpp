#include <iostream>
#include <vector>
#include <string>
#include <PluginManager.h>
#include <Player.h>
#include <PlayList.h>
#include <scx/Thread.hpp>
using namespace std;
using namespace scx;
using namespace mous;

Player* gPlayer = NULL;

void OnFinished()
{
    cout << "Finished!" << endl;
}

void OnPlaying()
{
    while (true) {
	if (gPlayer == NULL)
	    break;
	uint64_t ms = gPlayer->GetCurrentMs();
	cout << gPlayer->GetBitRate() << " kbps " <<
	    ms/1000/60 << ":" << ms/1000%60 << "." << ms%1000 << '\r' << flush;
	usleep(200*1000);
    }
}

int main(int argc, char** argv)
{
    bool paused = false;

    PluginManager mgr;
    mgr.LoadPluginDir("./plugins");

    /**
     * Dump all plugin path.
     */
    vector<string> pathList;
    mgr.GetPluginPath(pathList);
    for (size_t i = 0; i < pathList.size(); ++i) {
	cout << ">> " << pathList[i] << endl;
	const PluginInfo* info = mgr.GetPluginInfo(pathList[i]);
	cout << ">>>> " << info->author << endl;
	cout << ">>>> " << info->name << endl;
	cout << ">>>> " << info->description << endl;
	cout << ">>>> " << info->version << endl;
    }
    cout << endl;

    /**
     * Get all plugin instances.
     */
    vector<IDecoder*> decoderList;
    mgr.GetDecoders(decoderList);
    cout << ">> decoders count:" << decoderList.size() << endl;

    vector<IRenderer*> rendererList;
    mgr.GetRenderers(rendererList);
    cout << ">> renderers count:" << rendererList.size() << endl;
    cout << endl;

    /**
     * Check args enough.
     */
    if (argc < 2) {
	cout << "Usage:" << endl;
	cout << "mous-cli [-r] [file]" << endl;
	cout << "-r\tRepeat mode." << endl;
	return -1;
    }

    /**
     * Check plugins enough.
     */
    if (decoderList.empty() || rendererList.empty())
	return -2;

    /**
     * Setup player.
     */
    Player player;
    player.SigFinished.Connect(&OnFinished);
    player.SetRenderer(rendererList[0]);
    for (size_t i = 0; i < decoderList.size(); ++i) {
	player.AddDecoder(decoderList[i]);
    }

    /**
     * Begin to play.
     */
    player.Open(argv[1]);
    player.Play();
    Thread th;
    gPlayer = &player;
    th.Run(Function<void (void)>(&OnPlaying));

    char ch = ' ';
    while (ch != 'q') {
	cin >> ch;
	switch (ch) {
	    case 'q':
		player.Stop();
		player.Close();
		break;

	    case 's':
		paused = false;
		player.Stop();
		break;

	    case 'p':
		if (paused) {
		    player.Resume();
		    paused = false;
		} else {
		    player.Pause();
		    paused = true;
		}
		break;

	    case 'r':
		player.Play();
		break;
	}
    }

    th.Join();
    mgr.UnloadAllPlugins();

    return 0;
}
