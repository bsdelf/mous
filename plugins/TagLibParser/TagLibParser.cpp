#include "TagLibParser.h"
#include <stdlib.h>
#include <limits.h>
#include <iostream>
#include <scx/FileHelper.hpp>
#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>

TagLibParser::TagLibParser():
    m_pFileRef(NULL),
    m_pTag(NULL),
    m_pProp(NULL)
{
    m_Dumpers["mp3"] = &TagLibParser::DumpMp3Cover;
    m_Dumpers["m4a"] = &TagLibParser::DumpMp4Cover;
}

TagLibParser::~TagLibParser()
{
    m_Dumpers.clear();
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
    m_FileName = path;

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

    m_FileName.clear();
}

string WStringToStdString(const std::wstring& str)
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

bool TagLibParser::HasTag() const
{
    return (m_pTag != NULL) ? !m_pTag->isEmpty() : false;
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

bool TagLibParser::HasProperties() const
{
    return (m_pProp != NULL) ? true : false;
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

bool TagLibParser::CanEditTag() const
{
    return true;
}

bool TagLibParser::SaveTag()
{
    return m_pFileRef != NULL ? m_pFileRef->save() : false;
}

void TagLibParser::SetTitle(const string& title)
{
    if (m_pTag != NULL)
        m_pTag->setTitle(title.c_str());
}

void TagLibParser::SetArtist(const string& artist)
{
    if (m_pTag != NULL)
        m_pTag->setArtist(artist.c_str());
}

void TagLibParser::SetAlbum(const string& album)
{
    if (m_pTag != NULL)
        m_pTag->setAlbum(album.c_str());
}

void TagLibParser::SetComment(const string& comment)
{
    if (m_pTag != NULL)
        m_pTag->setComment(comment.c_str());
}

void TagLibParser::SetGenre(const string& genre)
{
    if (m_pTag != NULL)
        m_pTag->setGenre(genre.c_str());
}

void TagLibParser::SetYear(int32_t year)
{
    if (m_pTag != NULL)
        m_pTag->setYear(year);
}

void TagLibParser::SetTrack(int32_t track)
{
    if (m_pTag != NULL)
        m_pTag->setTrack(track);
}

bool TagLibParser::DumpCoverArt(char*& buf, size_t& len)
{
    if (m_FileName.empty())
        return false;

    const string& ext = scx::FileHelper::FileSuffix(m_FileName);
    cout << "ext:" << ext << endl;
    if (m_Dumpers.find(ext) == m_Dumpers.end())
        return false;

    buf = NULL;
    len = 0;
    m_Dumpers[ext](m_FileName, buf, len);

    return buf != NULL;
}

bool TagLibParser::StoreCoverArt(const char* buf, size_t len)
{
    if (m_FileName.empty())
        return false;

    return true;
}

void TagLibParser::DumpID3v2Cover(ID3v2::Tag* mp3Tag, char*& buf, size_t& len)
{
    if (mp3Tag == NULL){
        cout << "no id3v2 tag found!" << endl;
        return;
    } 

    ID3v2::FrameList frameList;
    ID3v2::AttachedPictureFrame* frame;

    const char* picId[] = { "APIC", "PIC" };
    for (int i = 0; i < 2; ++i) {
        frameList = mp3Tag->frameListMap()[picId[i]];
        if (!frameList.isEmpty()) {
            ID3v2::FrameList::ConstIterator iter = frameList.begin();
            for (; iter != frameList.end(); ++iter) {
                frame = static_cast<ID3v2::AttachedPictureFrame*>(*iter);
                cout << "type: " << (int) frame->type() << endl;
                cout << "mime: " << frame->mimeType().toCString() << endl;

                len = frame->picture().size();
                buf = new char[len];
                memcpy(buf, frame->picture().data(), len);
                return;
            }
        } else {
            cout << picId[i] << " not found!" << endl;
        }
    }
}

void TagLibParser::DumpMp3Cover(const string& path, char*& buf, size_t& len)
{
    TagLib::MPEG::File file(path.c_str());
    DumpID3v2Cover(file.ID3v2Tag(), buf, len);
}

void TagLibParser::DumpMp4Cover(const string& path, char*& buf, size_t& len)
{
    TagLib::MP4::File file(path.c_str());
    MP4::Tag* mp4tag = file.tag();

    if (mp4tag == NULL) {
        cout << "no mp4 tag found!" << endl;
        return;
    }

    MP4::ItemListMap::Iterator iter = mp4tag->itemListMap().find("covr");
    if (iter != mp4tag->itemListMap().end()) {
        MP4::CoverArtList list = iter->second.toCoverArtList();
        if (list.isEmpty()) {
            cout << "no cover art!" << endl;
        }
        cout << "CoverArtList count: " << list.size() << endl;

        cout << "type: " << list[0].format() << endl;

        len = list[0].data().size();
        buf = new char[len];
        memcpy(buf, list[0].data().data(), len);
        return;
    } else {
        cout << "\"covr\" not found!" << endl;
    }
}
