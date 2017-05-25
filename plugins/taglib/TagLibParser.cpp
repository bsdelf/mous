#include "TagLibParser.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <vector>

#include <scx/Conv.h>
#include <scx/FileHelper.h>
#include <scx/IconvHelper.h>
using namespace scx;

#include "CoverArt.h"

#include <taglib/mpegfile.h>

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

TagLibParser::TagLibParser()
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
    return { "*" };
}

ErrorCode TagLibParser::Open(const string& path)
{
    m_FileName = path;

    m_pFileRef = new FileRef(path.c_str(), true);//AudioProperties::);
    if (!m_pFileRef->isNull() && m_pFileRef->tag() != nullptr) {
        m_pTag = m_pFileRef->tag();
        m_pProp = m_pFileRef->audioProperties();
    }
    return ErrorCode::Ok;
}

void TagLibParser::Close()
{
    if (m_pFileRef != nullptr) {
        delete m_pFileRef;
        m_pFileRef = nullptr;
        m_pTag = nullptr;
        m_pProp = nullptr;
    }

    m_FileName.clear();
}

bool TagLibParser::HasTag() const
{
    return (m_pTag != nullptr) ? !m_pTag->isEmpty() : false;
}

string TagLibParser::Title() const
{
    if (m_pTag != nullptr) {
        return StringToStdString(m_pTag->title());
    } else {
        return "";
    }
}

string TagLibParser::Artist() const
{
    if (m_pTag != nullptr) {
        return StringToStdString(m_pTag->artist());
    } else {
        return "";
    }
}

string TagLibParser::Album() const
{
    if (m_pTag != nullptr) {
        return StringToStdString(m_pTag->album());
    } else {
        return "";
    }
}

string TagLibParser::Comment() const
{
    if (m_pTag != nullptr) {
        return StringToStdString(m_pTag->comment());
    } else {
        return "";
    }
}

string TagLibParser::Genre() const
{
    if (m_pTag != nullptr) {
        return StringToStdString(m_pTag->genre());
    } else {
        return "";
    }
}

int32_t TagLibParser::Year() const
{
    if (m_pTag != nullptr) {
        return m_pTag->year();
    } else {
        return -1;
    }
}

int32_t TagLibParser::Track() const
{
    if (m_pTag != nullptr) {
        return m_pTag->track();
    } else {
        return -1;
    }
}

bool TagLibParser::HasAudioProperty() const
{
    return (m_pProp != nullptr) ? true : false;
}

int32_t TagLibParser::Duration() const
{
    if (m_pProp != nullptr) {
        return m_pProp->length()*1000;
    } else {
        return 0;
    }
}

int32_t TagLibParser::BitRate() const
{
    if (m_pProp != nullptr) {
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
    const string& ext = ToLower(FileHelper::FileSuffix(m_FileName));
    if (ext == "mp3") {
        auto ref = dynamic_cast<TagLib::MPEG::File*>(m_pFileRef->file());
        if (ref == nullptr) {
            cout << "bad type" << endl;
            return false;
        }
        return ref->save(TagLib::MPEG::File::TagTypes::ID3v2, true, 3, true);
    } else {
        return m_pFileRef != nullptr ? m_pFileRef->save() : false;
    }
}

void TagLibParser::SetTitle(const string& title)
{
    if (m_pTag != nullptr)
        m_pTag->setTitle(title.c_str());
}

void TagLibParser::SetArtist(const string& artist)
{
    if (m_pTag != nullptr)
        m_pTag->setArtist(artist.c_str());
}

void TagLibParser::SetAlbum(const string& album)
{
    if (m_pTag != nullptr)
        m_pTag->setAlbum(album.c_str());
}

void TagLibParser::SetComment(const string& comment)
{
    if (m_pTag != nullptr)
        m_pTag->setComment(comment.c_str());
}

void TagLibParser::SetGenre(const string& genre)
{
    if (m_pTag != nullptr)
        m_pTag->setGenre(genre.c_str());
}

void TagLibParser::SetYear(int32_t year)
{
    if (m_pTag != nullptr)
        m_pTag->setYear(year);
}

void TagLibParser::SetTrack(int32_t track)
{
    if (m_pTag != nullptr)
        m_pTag->setTrack(track);
}

CoverFormat TagLibParser::DumpCoverArt(vector<char>& buf)
{
    if (m_FileName.empty())
        return CoverFormat::None;

    const string& ext = ToLower(FileHelper::FileSuffix(m_FileName));

    if (m_DumpHandlers.find(ext) != m_DumpHandlers.end())
        return m_DumpHandlers[ext](m_FileName, buf);
    else
        return CoverFormat::None;
}

bool TagLibParser::StoreCoverArt(CoverFormat fmt, const char* buf, size_t len)
{
    if (m_FileName.empty())
        return false;

    const string& ext = ToLower(FileHelper::FileSuffix(m_FileName));

    if (m_StoreHandlers.find(ext) != m_StoreHandlers.end())
        return m_StoreHandlers[ext](m_FileName, fmt, buf, len);
    else
        return false;
}
