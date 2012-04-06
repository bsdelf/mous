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
    virtual bool HasTag() const { return false; };
    virtual std::string GetTitle() const { return ""; } 
    virtual std::string GetArtist() const { return ""; }
    virtual std::string GetAlbum() const { return ""; }
    virtual std::string GetComment() const { return ""; }
    virtual std::string GetGenre() const { return ""; }
    virtual int32_t GetYear() const { return 0; }
    virtual int32_t GetTrack() const { return 0; };

    // tag edit calls
    virtual bool CanEditTag() const { return false; }
    virtual bool SaveTag() const { return false; }
    virtual void SetTitle(const std::string& title) { };
    virtual void SetArtist(const std::string& artist) { }; 
    virtual void SetAlbum(const std::string& album) { }; 
    virtual void SetComment(const std::string& comment) { }; 
    virtual void SetGenre(const std::string& genre) { }; 
    virtual void SetYear(int32_t year) { };
    virtual void SetTrack(int32_t track) { };

    // cover art calls
    virtual bool DumpCoverArt(char*& buf, size_t& len) { }
    virtual bool StoreCoverArt(const char* buf, size_t len) { }

    // property related calls
    virtual bool HasProperties() const = 0;
    virtual int32_t GetDuration() const = 0;
    virtual int32_t GetBitRate() const = 0;

    // reimplement this to provide options
    virtual bool GetOptions(std::vector<const BaseOption*>& list) const
    {
        list.clear();
        return false; 
    };
};

}

#endif
