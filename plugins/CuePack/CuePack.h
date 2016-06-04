#include <plugin/IMediaPack.h>
#include <libcue.h>

using namespace std;
using namespace mous;

class CuePack: public IMediaPack
{
public:
    CuePack();
    virtual ~CuePack();

    virtual vector<string> FileSuffix() const;

    virtual void DumpMedia(const string& path, deque<MediaItem>& list,
	    const unordered_map<string, IMediaPack*>* pMap) const;

    virtual void DumpStream(const string& stream, deque<MediaItem>& list,
	    const unordered_map<string, IMediaPack*>* pMap) const;

private:
    void DumpCue(const string& dir, Cd* cd, deque<MediaItem>& list) const;
};
