#ifndef TAGLIBPARSER_H
#define TAGLIBPARSER_H

#include <plugin/ITagParser.h>
using namespace mous;

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/audioproperties.h>
using namespace TagLib;

#include <unordered_map>
#include <vector>
using namespace std;

namespace TagLib
{
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

    virtual bool CanEdit() const;
    virtual bool Save();
    virtual void SetTitle(const string& title);
    virtual void SetArtist(const string& artist); 
    virtual void SetAlbum(const string& album); 
    virtual void SetComment(const string& comment); 
    virtual void SetGenre(const string& genre); 
    virtual void SetYear(int32_t year);
    virtual void SetTrack(int32_t track);

    virtual EmCoverFormat DumpCoverArt(vector<char>& buf);
    virtual bool StoreCoverArt(EmCoverFormat fmt, const char* buf, size_t len);

    virtual bool HasAudioProperty() const;
    virtual int32_t Duration() const;
    virtual int32_t BitRate() const;

private:
    typedef EmCoverFormat (*FnDumpCover)(const string& path, vector<char>& buf);
    typedef bool (*FnStoreCover)(const string& path, EmCoverFormat fmt, const char* buf, size_t len);

private:
    std::string m_FileName;

    FileRef* m_pFileRef = nullptr;
    Tag* m_pTag = nullptr;
    AudioProperties* m_pProp = nullptr;

    std::unordered_map<std::string, FnDumpCover> m_DumpHandlers;
    std::unordered_map<std::string, FnStoreCover> m_StoreHandlers;
};

#endif
