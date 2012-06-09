#ifndef MOUS_ITAGPARSER_H
#define MOUS_ITAGPARSER_H

#include <inttypes.h>
#include <vector>
#include <string>
#include <util/ErrorCode.h>
#include <util/Option.h>

namespace mous {

namespace CoverFormat {
    enum e
    {
        None = 0,
        JPEG,
        PNG,
    };
}
typedef CoverFormat::e EmCoverFormat;

class ITagParser
{
public:
    

public:
    virtual ~ITagParser() { }

    virtual std::vector<std::string> FileSuffix() const = 0;
    
    virtual EmErrorCode Open(const std::string& path) = 0;
    virtual void Close() = 0;

    // read tag
    virtual bool HasTag() const = 0;
    virtual std::string Title() const = 0; 
    virtual std::string Artist() const = 0;
    virtual std::string Album() const = 0;
    virtual std::string Comment() const = 0;
    virtual std::string Genre() const = 0;
    virtual int32_t Year() const = 0;
    virtual int32_t Track() const = 0;

    // edit tag
    virtual bool CanEdit() const { return false; };
    virtual bool Save() { return false; };
    virtual void SetTitle(const std::string& title) { };
    virtual void SetArtist(const std::string& artist) { };
    virtual void SetAlbum(const std::string& album) { };
    virtual void SetComment(const std::string& comment) { };
    virtual void SetGenre(const std::string& genre) { };
    virtual void SetYear(int32_t year) { };
    virtual void SetTrack(int32_t track) { };

    // cover art
    virtual EmCoverFormat DumpCoverArt(std::vector<char>& buf) { buf.clear(); return CoverFormat::None; };
    virtual bool StoreCoverArt(EmCoverFormat fmt, const char* buf, size_t len) { return false; };

    // audio property
    virtual bool HasAudioProperty() const { return false; }
    virtual int32_t Duration() const { return -1; }
    virtual int32_t BitRate() const { return -1; }

    // reimplement this to provide options
    virtual bool Options(std::vector<const BaseOption*>& list) const
    {
        list.clear();
        return false; 
    };
};

}

#endif
