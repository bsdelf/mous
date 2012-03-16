#ifndef TAGLIBPARSER_H
#define TAGLIBPARSER_H

#include <mous/ITagParser.h>
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

    virtual vector<string> FileSuffix() const;
    
    virtual EmErrorCode Open(const string& path);
    virtual void Close();

    virtual string Title() const;
    virtual string Artist() const;
    virtual string Album() const;
    virtual string Comment() const;
    virtual string Genre() const;
    virtual int32_t Year() const;
    virtual int32_t Track() const;

    virtual int32_t Duration() const;
    virtual int32_t BitRate() const;

    virtual void SetTitle(const string& title);
    virtual void SetArtist(const string& artist); 
    virtual void SetAlbum(const string& album); 
    virtual void SetComment(const string& comment); 
    virtual void SetGenre(const string& genre); 
    virtual void SetYear(int32_t year);
    virtual void SetTrack(int32_t track);

    virtual bool HasTag();
    virtual bool HasProperties();

private:
    FileRef* m_pFileRef;
    Tag* m_pTag;
    AudioProperties* m_pProp;
};

#endif
