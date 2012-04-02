#include "TagLibParser.h"

TagLibParser::TagLibParser():
    m_pFileRef(NULL),
    m_pTag(NULL),
    m_pProp(NULL)
{

}

TagLibParser::~TagLibParser()
{
    Close();
}

vector<string> TagLibParser::GetFileSuffix() const
{
    vector<string> list;
    list.clear();
    list.push_back("*");
    return list;
}

EmErrorCode TagLibParser::Open(const string& path)
{
    m_pFileRef = new FileRef(path.c_str(), true);//AudioProperties::);
    if (!m_pFileRef->isNull() && m_pFileRef->tag() != NULL) {
        m_pTag = m_pFileRef->tag();
        m_pProp = m_pFileRef->audioProperties();
    }
    return ErrorCode::Ok;
}

void TagLibParser::Close()
{
    if (m_pFileRef != NULL) {
        delete m_pFileRef;
        m_pFileRef = NULL;
        m_pTag = NULL;
        m_pProp = NULL;
    }
}

string WStringToStdString(const wstring& str)
{
    const size_t buflen = (str.size()+1) * MB_LEN_MAX;
    char* buf = new char[buflen];
    size_t ret = wcstombs(buf, str.c_str(), buflen);
    if (ret != (size_t)-1) {
        string target(buf, ret);
        delete[] buf;
        return target;
    } else {
        delete[] buf;
        return "";
    }
}

string StringToStdString(const TagLib::String& str)
{
    return ( str.isLatin1() || str.isAscii() ) ? 
        str.to8Bit() : WStringToStdString(str.toWString());
}

string TagLibParser::GetTitle() const
{
    if (m_pTag != NULL) {
        //return m_pTag->title().to8Bit();
        return StringToStdString(m_pTag->title());
    } else {
        return "";
    }
}

string TagLibParser::GetArtist() const
{
    if (m_pTag != NULL) {
        //return m_pTag->artist().to8Bit();
        return StringToStdString(m_pTag->artist());
    } else {
        return "";
    }
}

string TagLibParser::GetAlbum() const
{
    if (m_pTag != NULL) {
        //return m_pTag->album().to8Bit();
        return StringToStdString(m_pTag->album());
    } else {
        return "";
    }
}

string TagLibParser::GetComment() const
{
    if (m_pTag != NULL) {
        //return m_pTag->comment().to8Bit();
        return StringToStdString(m_pTag->comment());
    } else {
        return "";
    }
}

string TagLibParser::GetGenre() const
{
    if (m_pTag != NULL) {
        //return m_pTag->genre().to8Bit();
        return StringToStdString(m_pTag->genre());
    } else {
        return "";
    }
}

int32_t TagLibParser::GetYear() const
{
    if (m_pTag != NULL) {
        return m_pTag->year();
    } else {
        return -1;
    }
}

int32_t TagLibParser::GetTrack() const
{
    if (m_pTag != NULL) {
        return m_pTag->track();
    } else {
        return -1;
    }
}

int32_t TagLibParser::GetDuration() const
{
    if (m_pProp != NULL) {
        return m_pProp->length()*1000;
    } else {
        return 0;
    }
}

int32_t TagLibParser::GetBitRate() const
{
    if (m_pProp != NULL) {
        return m_pProp->bitrate();
    } else {
        return 0;
    }
}

void TagLibParser::SetTitle(const string& title)
{

}

void TagLibParser::SetArtist(const string& artist)
{

}

void TagLibParser::SetAlbum(const string& album)
{

}

void TagLibParser::SetComment(const string& comment)
{

}

void TagLibParser::SetGenre(const string& genre)
{

}

void TagLibParser::SetYear(int32_t year)
{

}

void TagLibParser::SetTrack(int32_t track)
{

}

bool TagLibParser::HasTag() const
{
    return (m_pTag != NULL) ? !m_pTag->isEmpty() : false;
}

bool TagLibParser::HasProperties() const
{
    return (m_pProp != NULL) ? true : false;
}
