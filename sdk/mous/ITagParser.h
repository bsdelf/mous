#ifndef MOUS_TAGPARSER_H
#define MOUS_TAGPARSER_H

#include <inttypes.h>
#include <string>
#include "ErrorCode.h"

namespace mous {

class ITagParser
{
public:
    virtual ~ITagParser()
    {

    }

    virtual std::string GetTitle() = 0;
    virtual std::string GetArtist() = 0;
    virtual std::string GetAlbum() = 0;
    virtual std::string GetComment() = 0;
    virtual std::string GetGenre() = 0;
    virtual int32_t GetYear() = 0;
    virtual int32_t GetTrack() = 0;

    virtual EmErrorCode SetTitle(const std::string& title) = 0;
    virtual EmErrorCode SetArtist(const std::string& artist) = 0; 
    virtual EmErrorCode SetAlbum(const std::string& album) = 0; 
    virtual EmErrorCode SetComment(const std::string& comment) = 0; 
    virtual EmErrorCode SetGenre(const std::string& genre) = 0; 
    virtual EmErrorCode SetYear(int32_t year) = 0;
    virtual EmErrorCode SetTrack(int32_t track) = 0;
};

}
