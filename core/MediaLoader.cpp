#include "MediaLoader.h"
#include <mous/ITagParser.h>
using namespace std;
using namespace mous;

MediaLoader::MediaLoader()
{

}

MediaLoader::~MediaLoader()
{

}

EmErrorCode MediaLoader::LoadMedia(const std::string& path, PlayList& list)
{
    return ErrorCode::Ok;
}
