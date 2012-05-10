#ifndef TAGLIBPARSER_H
#define TAGLIBPARSER_H

#include <plugin/ITagParser.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/audioproperties.h>
#include <map>
using namespace std;
using namespace TagLib;
using namespace mous;

namespace TagLib{
namespace ID3v2
{
    class Tag;
}
}

class TagLibParser: public ITagParser
{
public:
    TagLibParser();
    virtual ~TagLibParser();

    virtual vector<string> FileSuffix() const;
    
    virtual EmErrorCode Open(const string& path);
    virtual void Close();

    virtual bool HasTag() const;
    virtual string Title() const;
    virtual string Artist() const;
    virtual string Album() const;
    virtual string Comment() const;
    virtual string Genre() const;
    virtual int32_t Year() const;
    virtual int32_t Track() const;

    virtual bool CanEditTag() const;
    virtual bool SaveTag();
    virtual void SetTitle(const string& title);
    virtual void SetArtist(const string& artist); 
    virtual void SetAlbum(const string& album); 
    virtual void SetComment(const string& comment); 
    virtual void SetGenre(const string& genre); 
    virtual void SetYear(int32_t year);
    virtual void SetTrack(int32_t track);

    virtual bool DumpCoverArt(char*& buf, size_t& len);
    virtual bool StoreCoverArt(const char* buf, size_t len);

    virtual bool HasProperties() const;
    virtual int32_t Duration() const;
    virtual int32_t BitRate() const;

private:
    typedef void (*FnDumpCover)(const string& path, char*& buf, size_t& len);
    static void DumpID3v2Cover(ID3v2::Tag* mp3Tag, char*& buf, size_t& len);
    static void DumpMp3Cover(const string& path, char*& buf, size_t& len);
    static void DumpMp4Cover(const string& path, char*& buf, size_t& len);

private:
    std::string m_FileName;

    FileRef* m_pFileRef;
    Tag* m_pTag;
    AudioProperties* m_pProp;

    std::map<std::string, FnDumpCover> m_Dumpers;
};

#endif
