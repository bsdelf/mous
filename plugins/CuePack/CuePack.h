#include <mous/IMediaPack.h>
extern "C" {
#include <libcue-1.4/libcue/libcue.h>
#include <libcue-1.4/libcue/cd.h>
}
using namespace std;
using namespace mous;

class CuePack: public IMediaPack
{
public:
    CuePack();
    virtual ~CuePack();

    virtual void GetFileSuffix(std::vector<std::string>& list) const;

    virtual void DumpMedia(const std::string& path, std::deque<MediaItem*>& list,
	    const std::map<std::string, IMediaPack*>* pMap) const;

    virtual void DumpStream(const std::string& stream, std::deque<MediaItem*>& list,
	    const std::map<std::string, IMediaPack*>* pMap) const;
};
