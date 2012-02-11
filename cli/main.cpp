#include <iostream>
#include <vector>
#include <string>
#include <PluginManager.h>
#include <Player.h>
#include <PlayList.h>
using namespace std;
using namespace mous;

int main(int argc, char** argv)
{
    char ch = ' ';
    bool paused = false;
    PluginManager mgr;

    mgr.LoadPluginDir("./plugins");

    /**
     * Dump all plugin path.
     */
    vector<string> list;
    mgr.GetPluginPath(list);
    for (size_t i = 0; i < list.size(); ++i) {
	cout << ">> " << list[i] << endl;
	const PluginInfo* info = mgr.GetPluginInfo(list[i]);
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
     * Check args
     */
    if (argc < 2) {
	cout << "Usage:" << endl;
	cout << "mous-cli [-r] [file]" << endl;
	cout << "-r\tRepeat mode." << endl;
	return -1;
    }

    Player player;

    /**
     * Setup player.
     */
    player.Open(argv[1]);
    player.Play();

    while (ch != 'q') {
	cin >> ch;
	switch (ch) {
	    case 'q':
		player.Stop();
		player.Close();
		break;

	    case 's':
		player.Stop();
		cout << "done" << endl;
		player.Stop();
		cout << "done" << endl;
		player.Close();
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

    mgr.UnloadAllPlugins();

    return 0;
}
