#ifndef MOUS_ITAGPARSER_H
#define MOUS_ITAGPARSER_H

#include <inttypes.h>
#include <vector>
#include <string>
#include "ErrorCode.h"

namespace mous {

class ITagParser
{
public:
    virtual ~ITagParser()
    {

    }

    virtual void GetFileSuffix(std::vector<std::string>& list) const = 0;
    
    virtual EmErrorCode Open(const std::string& path) = 0;
    virtual void Close() = 0;

    virtual std::string GetTitle() = 0;
    virtual std::string GetArtist() = 0;
    virtual std::string GetAlbum() = 0;
    virtual std::string GetComment() = 0;
    virtual std::string GetGenre() = 0;
    virtual int32_t GetYear() = 0;
    virtual int32_t GetTrack() = 0;

    virtual void SetTitle(const std::string& title) = 0;
    virtual void SetArtist(const std::string& artist) = 0; 
    virtual void SetAlbum(const std::string& album) = 0; 
    virtual void SetComment(const std::string& comment) = 0; 
    virtual void SetGenre(const std::string& genre) = 0; 
    virtual void SetYear(int32_t year) = 0;
    virtual void SetTrack(int32_t track) = 0;

    virtual bool IsEmpty() = 0;
};

}

#endif
