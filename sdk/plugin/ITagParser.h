#ifndef MOUS_ITAGPARSER_H
#define MOUS_ITAGPARSER_H

#include <inttypes.h>
#include <vector>
#include <string>
#include <util/ErrorCode.h>
#include <util/Option.h>

namespace mous {

class ITagParser
{
public:
    virtual ~ITagParser() { }

    virtual std::vector<std::string> GetFileSuffix() const = 0;
    
    virtual EmErrorCode Open(const std::string& path) = 0;
    virtual void Close() = 0;

    // tag related calls
    virtual bool HasTag() const = 0;
    virtual std::string GetTitle() const = 0; 
    virtual std::string GetArtist() const = 0;
    virtual std::string GetAlbum() const = 0;
    virtual std::string GetComment() const = 0;
    virtual std::string GetGenre() const = 0;
    virtual int32_t GetYear() const = 0;
    virtual int32_t GetTrack() const = 0;

    // tag edit calls
    virtual bool CanEditTag() const = 0;
    virtual bool SaveTag() = 0;
    virtual void SetTitle(const std::string& title) = 0;
    virtual void SetArtist(const std::string& artist) = 0;
    virtual void SetAlbum(const std::string& album) = 0;
    virtual void SetComment(const std::string& comment) = 0;
    virtual void SetGenre(const std::string& genre) = 0;
    virtual void SetYear(int32_t year) = 0;;
    virtual void SetTrack(int32_t track) = 0;;

    // cover art calls
    virtual bool DumpCoverArt(char*& buf, size_t& len) = 0;
    virtual bool StoreCoverArt(const char* buf, size_t len) = 0;

    // property related calls
    virtual bool HasProperties() const { return false; }
    virtual int32_t GetDuration() const { return -1; }
    virtual int32_t GetBitRate() const { return -1; }

    // reimplement this to provide options
    virtual bool GetOptions(std::vector<const BaseOption*>& list) const
    {
        list.clear();
        return false; 
    };
};

}

#endif
