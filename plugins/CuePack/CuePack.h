#include <plugin/IMediaPack.h>
extern "C" {
#include <libcue-1.4/libcue/libcue.h>
#include <libcue-1.4/libcue/cd.h>
}
using namespace std;
using namespace mous;

struct Cd;

class CuePack: public IMediaPack
{
public:
    CuePack();
    virtual ~CuePack();

    virtual vector<string> FileSuffix() const;

    virtual void DumpMedia(const string& path, deque<MediaItem>& list,
	    const map<string, IMediaPack*>* pMap) const;

    virtual void DumpStream(const string& stream, deque<MediaItem>& list,
	    const map<string, IMediaPack*>* pMap) const;

private:
    void DumpCue(const string& dir, Cd* cd, deque<MediaItem>& list) const;
};
