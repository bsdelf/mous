#ifndef TAGLIBPARSER_H
#define TAGLIBPARSER_H

#include <mous/ITagParser.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
using namespace std;
using namespace TagLib;
using namespace mous;


class TagLibParser: public ITagParser
{
public:
    TagLibParser();
    virtual ~TagLibParser();

    virtual void GetFileSuffix(vector<string>& list) const;
    
    virtual EmErrorCode Open(const string& path);
    virtual void Close();

    virtual string GetTitle();
    virtual string GetArtist();
    virtual string GetAlbum();
    virtual string GetComment();
    virtual string GetGenre();
    virtual int32_t GetYear();
    virtual int32_t GetTrack();

    virtual void SetTitle(const string& title);
    virtual void SetArtist(const string& artist); 
    virtual void SetAlbum(const string& album); 
    virtual void SetComment(const string& comment); 
    virtual void SetGenre(const string& genre); 
    virtual void SetYear(int32_t year);
    virtual void SetTrack(int32_t track);

    virtual bool IsEmpty();

private:
    FileRef* m_pFileRef;
    Tag* m_pTag;
};

#endif
