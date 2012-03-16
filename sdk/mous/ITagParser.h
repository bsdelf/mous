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
    virtual ~ITagParser() { }

    virtual std::vector<std::string> FileSuffix() const = 0;
    
    virtual EmErrorCode Open(const std::string& path) = 0;
    virtual void Close() = 0;

    virtual std::string Title() const = 0;
    virtual std::string Artist() const = 0;
    virtual std::string Album() const = 0;
    virtual std::string Comment() const = 0;
    virtual std::string Genre() const = 0;
    virtual int32_t Year() const = 0;
    virtual int32_t Track() const = 0;

    virtual int32_t Duration() const = 0;
    virtual int32_t BitRate() const = 0;

    virtual void SetTitle(const std::string& title) = 0;
    virtual void SetArtist(const std::string& artist) = 0; 
    virtual void SetAlbum(const std::string& album) = 0; 
    virtual void SetComment(const std::string& comment) = 0; 
    virtual void SetGenre(const std::string& genre) = 0; 
    virtual void SetYear(int32_t year) = 0;
    virtual void SetTrack(int32_t track) = 0;

    virtual bool HasTag() = 0;
    virtual bool HasProperties() = 0;
};

}

#endif
