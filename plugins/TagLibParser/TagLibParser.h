#ifndef TAGLIBPARSER_H
#define TAGLIBPARSER_H

#include <plugin/ITagParser.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/audioproperties.h>
using namespace std;
using namespace TagLib;
using namespace mous;

class TagLibParser: public ITagParser
{
public:
    TagLibParser();
    virtual ~TagLibParser();

    virtual vector<string> GetFileSuffix() const;
    
    virtual EmErrorCode Open(const string& path);
    virtual void Close();

    virtual string GetTitle() const;
    virtual string GetArtist() const;
    virtual string GetAlbum() const;
    virtual string GetComment() const;
    virtual string GetGenre() const;
    virtual int32_t GetYear() const;
    virtual int32_t GetTrack() const;

    virtual int32_t GetDuration() const;
    virtual int32_t GetBitRate() const;

    virtual void SetTitle(const string& title);
    virtual void SetArtist(const string& artist); 
    virtual void SetAlbum(const string& album); 
    virtual void SetComment(const string& comment); 
    virtual void SetGenre(const string& genre); 
    virtual void SetYear(int32_t year);
    virtual void SetTrack(int32_t track);

    virtual bool HasTag() const;
    virtual bool HasProperties() const;

private:
    FileRef* m_pFileRef;
    Tag* m_pTag;
    AudioProperties* m_pProp;
};

#endif
