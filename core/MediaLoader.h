#ifndef MOUS_MEDIALOADER_H
#define MOUS_MEDIALOADER_H

#include <map>
#include <string>
#include <mous/ErrorCode.h>
#include "PlayList.h"

namespace mous {

class ITagParser;

class MediaLoader
{
public:
    MediaLoader();
    ~MediaLoader();

    EmErrorCode LoadMedia(const std::string& path, PlayList& list);

private:
    EmErrorCode TryUnpack();
    EmErrorCode TryParseTag();

private:
    std::map<std::string, ITagParser*> m_TagParserMap;
    typedef std::map<std::string, ITagParser*>::iterator TagParserMapIter;
    typedef std::pair<std::string, ITagParser*> TagParserMapPair;
};

}

#endif
