#include "TagLibParser.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <iostream>
#include <vector>

#include <scx/Conv.hpp>
#include <scx/FileHelper.hpp>
#include <scx/IconvHelper.hpp>
using namespace scx;

#include "CoverArt.h"

static string StringToStdString(const String& str)
{
    if (str.isLatin1()) {
        return str.to8Bit();
    } else {
        string stdStr;
        const ByteVector& v = str.data(String::UTF16BE);
        if (IconvHelper::ConvFromTo("UTF-16BE", "UTF-8", v.data(), v.size(), stdStr))
            return stdStr;
        else
            return str.to8Bit();
    }
}

TagLibParser::TagLibParser():
    m_pFileRef(NULL),
    m_pTag(NULL),
    m_pProp(NULL)
{
    m_DumpHandlers["mp3"] = &DumpMp3Cover;
    m_DumpHandlers["m4a"] = &DumpMp4Cover;

    m_StoreHandlers["mp3"] = &StoreMp3Cover;
    m_StoreHandlers["m4a"] = &StoreMp4Cover;
}

TagLibParser::~TagLibParser()
{
    m_DumpHandlers.clear();
    Close();
}

vector<string> TagLibParser::FileSuffix() const
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

bool TagLibParser::HasTag() const
{
    return (m_pTag != NULL) ? !m_pTag->isEmpty() : false;
}

string TagLibParser::Title() const
{
    if (m_pTag != NULL) {
        return StringToStdString(m_pTag->title());
    } else {
        return "";
    }
}

string TagLibParser::Artist() const
{
    if (m_pTag != NULL) {
        return StringToStdString(m_pTag->artist());
    } else {
        return "";
    }
}

string TagLibParser::Album() const
{
    if (m_pTag != NULL) {
        return StringToStdString(m_pTag->album());
    } else {
        return "";
    }
}

string TagLibParser::Comment() const
{
    if (m_pTag != NULL) {
        return StringToStdString(m_pTag->comment());
    } else {
        return "";
    }
}

string TagLibParser::Genre() const
{
    if (m_pTag != NULL) {
        return StringToStdString(m_pTag->genre());
    } else {
        return "";
    }
}

int32_t TagLibParser::Year() const
{
    if (m_pTag != NULL) {
        return m_pTag->year();
    } else {
        return -1;
    }
}

int32_t TagLibParser::Track() const
{
    if (m_pTag != NULL) {
        return m_pTag->track();
    } else {
        return -1;
    }
}

bool TagLibParser::HasAudioProperty() const
{
    return (m_pProp != NULL) ? true : false;
}

int32_t TagLibParser::Duration() const
{
    if (m_pProp != NULL) {
        return m_pProp->length()*1000;
    } else {
        return 0;
    }
}

int32_t TagLibParser::BitRate() const
{
    if (m_pProp != NULL) {
        return m_pProp->bitrate();
    } else {
        return 0;
    }
}

bool TagLibParser::CanEdit() const
{
    return true;
}

bool TagLibParser::Save()
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

EmCoverFormat TagLibParser::DumpCoverArt(vector<char>& buf)
{
    if (m_FileName.empty())
        return CoverFormat::None;

    const string& ext = ToLower(FileHelper::FileSuffix(m_FileName));
    cout << "DumpCoverArt ext:" << ext << endl;

    if (m_DumpHandlers.find(ext) != m_DumpHandlers.end())
        return m_DumpHandlers[ext](m_FileName, buf);
    else
        return CoverFormat::None;
}

bool TagLibParser::StoreCoverArt(EmCoverFormat fmt, const char* buf, size_t len)
{
    if (m_FileName.empty())
        return false;

    const string& ext = ToLower(FileHelper::FileSuffix(m_FileName));
    cout << "StoreCoverArt ext:" << ext << endl;

    if (m_StoreHandlers.find(ext) != m_StoreHandlers.end())
        return m_StoreHandlers[ext](m_FileName, fmt, buf, len);
    else
        return false;
}
