#include <plugin/ISheetParser.h>
#include <libcue.h>

using namespace std;
using namespace mous;

class CueSheetParser: public ISheetParser
{
public:
    CueSheetParser();
    virtual ~CueSheetParser();

    virtual vector<string> FileSuffix() const;

    virtual void DumpMedia(const string& path, deque<MediaItem>& list) const;

    virtual void DumpStream(const string& stream, deque<MediaItem>& list) const;

private:
    void DumpCue(const string& dir, Cd* cd, deque<MediaItem>& list) const;
};
