#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <vector>
#include <map>

#include <scx/Conv.h>
#include <scx/FileHelper.h>
#include <scx/IconvHelper.h>
using namespace scx;

#include <plugin/TagParserProto.h>
using namespace mous;

#include <taglib/mpegfile.h>
#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <taglib/audioproperties.h>
using namespace TagLib;

#include "CoverArt.h"

namespace TagLib {
    namespace ID3v2{
        class Tag;
    }
}

namespace {
    struct Self {
        std::string file_name;

        FileRef* file_ref = nullptr;
        Tag* tag = nullptr;
        AudioProperties* props = nullptr;

        std::map<std::string, CoverFormat(*)(const string&, char**, uint32_t*)> dump_funcs;
        std::map<std::string, bool(*)(const string&, CoverFormat, const char*, size_t)> store_funcs;

        std::string title;
        std::string artist;
        std::string album;
        std::string comment;
        std::string genre;
    };
}

static string StringTostd_string(const String& str) {
    if (str.isLatin1()) {
        return str.to8Bit();
    } 
    string std_str;
    const ByteVector& v = str.data(String::UTF16BE);
    if (IconvHelper::ConvFromTo("UTF-16BE", "UTF-8", v.data(), v.size(), std_str)) {
        return std_str;
    } else {
        return str.to8Bit();
    }
}

static void* Create() {
    auto self = new Self();
    self->dump_funcs = {
        { "mp3", DumpMp3Cover },
        { "m4a", DumpMp4Cover}
    };
    self->store_funcs = {
        { "mp3", StoreMp3Cover },
        { "m4a", StoreMp4Cover }
    };
    return self;
}

static void Destroy(void* ptr) {
    Close(ptr);
    delete SELF;
}

static ErrorCode Open(void* ptr, const char* path) {
    SELF->file_name = path;
    SELF->file_ref = new FileRef(path, true);//AudioProperties::);
    if (!SELF->file_ref->isNull() && SELF->file_ref->tag() != nullptr) {
        SELF->tag = SELF->file_ref->tag();
        SELF->props = SELF->file_ref->audioProperties();
    }
    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (SELF->file_ref) {
        delete SELF->file_ref;
        SELF->file_ref = nullptr;
        SELF->tag = nullptr;
        SELF->props = nullptr;
    }

    SELF->file_name.clear();
}

static bool HasTag(void* ptr) {
    return SELF->tag != nullptr;
}

static const char* GetTitle(void* ptr) {
    if (!SELF->tag) {
        return nullptr;
    }
    SELF->title = StringTostd_string(SELF->tag->title());
    return SELF->title.c_str();
}

static const char* GetArtist(void* ptr)
{
    if (!SELF->tag) {
        return nullptr;
    }
    SELF->artist = StringTostd_string(SELF->tag->artist());
    return SELF->artist.c_str();
}

static const char* GetAlbum(void* ptr)
{
    if (!SELF->tag) {
        return nullptr;
    }
    SELF->album = StringTostd_string(SELF->tag->album());
    return SELF->album.c_str();
}

static const char* GetComment(void* ptr) {
    if (!SELF->tag) {
        return nullptr;
    }
    SELF->comment = StringTostd_string(SELF->tag->comment());
    return SELF->comment.c_str();
}

static const char* GetGenre(void* ptr) {
    if (!SELF->tag) {
        return nullptr;
    }
    SELF->genre = StringTostd_string(SELF->tag->genre());
    return SELF->genre.c_str();
}

static int32_t GetYear(void* ptr)
{
    if (!SELF->tag) {
        return -1;
    }
    return SELF->tag->year();
}

static int32_t GetTrack(void* ptr)
{
    if (!SELF->tag) {
        return -1;
    }
    return SELF->tag->track();
}

static bool HasAudioProperties(void* ptr)
{
    return SELF->props != nullptr;
}

static int32_t GetDuration(void* ptr)
{
    if (!SELF->props) {
        return 0;
    }
    return SELF->props->length()*1000;
}

static int32_t GetBitRate(void* ptr)
{
    if (!SELF->props) {
        return 0;
    }
    return SELF->props->bitrate();
}

static bool CanEdit(void* ptr) {
    return true;
}

static bool Save(void* ptr)
{
    const string& ext = ToLower(FileHelper::FileSuffix(SELF->file_name));
    if (ext == "mp3") {
        auto ref = dynamic_cast<TagLib::MPEG::File*>(SELF->file_ref->file());
        if (!ref) {
            cout << "bad type" << endl;
            return false;
        }
        return ref->save(TagLib::MPEG::File::TagTypes::ID3v2, true, 3, true);
    } else {
        return SELF->file_ref != nullptr ? SELF->file_ref->save() : false;
    }
}

static void SetTitle(void* ptr, const char* title)
{
    if (SELF->tag) {
        SELF->tag->setTitle(title);
    }
}

static void SetArtist(void* ptr, const char* artist)
{
    if (SELF->tag) {
        SELF->tag->setArtist(artist);
    }
}

static void SetAlbum(void* ptr, const char* album)
{
    if (SELF->tag) {
        SELF->tag->setAlbum(album);
    }
}

static void SetComment(void* ptr, const char* comment)
{
    if (SELF->tag) {
        SELF->tag->setComment(comment);
    }
}

static void SetGenre(void* ptr, const char* genre)
{
    if (SELF->tag) {
        SELF->tag->setGenre(genre);
    }
}

static void SetYear(void* ptr, int32_t year)
{
    if (SELF->tag) {
        SELF->tag->setYear(year);
    }
}

static void SetTrack(void* ptr, int32_t track)
{
    if (SELF->tag) {
        SELF->tag->setTrack(track);
    }
}

static CoverFormat DumpCoverArt(void* ptr, char** out, uint32_t* length)
{
    if (SELF->file_name.empty()) {
        return CoverFormat::None;
    }
    const string& ext = ToLower(FileHelper::FileSuffix(SELF->file_name));
    if (SELF->dump_funcs.find(ext) != SELF->dump_funcs.end()) {
        return SELF->dump_funcs[ext](SELF->file_name, out, length);
    } else {
        return CoverFormat::None;
    }
}

static bool StoreCoverArt(void* ptr, CoverFormat fmt, const char* data, size_t length)
{
    if (SELF->file_name.empty()) {
        return false;
    }
    const string& ext = ToLower(FileHelper::FileSuffix(SELF->file_name));
    if (SELF->store_funcs.find(ext) != SELF->store_funcs.end()) {
        return SELF->store_funcs[ext](SELF->file_name, fmt, data, length);
    } else {
        return false;
    }
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

static const char** GetSuffixes(void* ptr) {
    static const char* suffixes[] { "*", nullptr };
    return suffixes;
}
