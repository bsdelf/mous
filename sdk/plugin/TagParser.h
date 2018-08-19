#pragma once

#include <inttypes.h>
#include <string.h>
#include <vector>
#include <string>
#include <util/ErrorCode.h>
#include <util/Option.h>
#include "Interface.h"
#include "InterfaceWrapper.h"

namespace mous {


class TagParser
{
    COPY_INTERFACE_WRAPPER_COMMON(TagParser, TagParserInterface, "MousGetTagParserInterface");

public:
    ErrorCode Open(const std::string& path) {
        return m_interface->open(m_data, path.c_str());
    }

    void Close() {
        return m_interface->close(m_data);
    }

    bool HasTag() const {
        return m_interface->has_tag(m_data);
    }

    std::string Title() const {
        auto str = m_interface->get_title(m_data);
        if (!str) {
            return {};
        }
        return str;
    }

    std::string Artist() const {
        auto str = m_interface->get_artist(m_data);
        if (!str) {
            return {};
        }
        return str;
    }

    std::string Album() const {
        auto str = m_interface->get_album(m_data);
        if (!str) {
            return {};
        }
        return str;
    }

    std::string Comment() const {
        auto str = m_interface->get_comment(m_data);
        if (!str) {
            return {};
        }
        return str;
    }

    std::string Genre() const {
        auto str = m_interface->get_genre(m_data);
        if (!str) {
            return {};
        }
        return str;
    }

    int32_t Year() const {
        return m_interface->get_year(m_data);
    }

    int32_t Track() const {
        return m_interface->get_track(m_data);
    }
    
    bool CanEdit() const {
        return m_interface->can_edit(m_data);
    }

    bool Save() {
        return m_interface->save(m_data);
    }

    void SetTitle(const std::string& title) {
        return m_interface->set_title(m_data, title.c_str());
    }

    void SetArtist(const std::string& artist) {
        return m_interface->set_artist(m_data, artist.c_str());
    }

    void SetAlbum(const std::string& album) {
        return m_interface->set_album(m_data, album.c_str());
    }

    void SetComment(const std::string& comment) {
        return m_interface->set_comment(m_data, comment.c_str());
    }

    void SetGenre(const std::string& genre) {
        return m_interface->set_genre(m_data, genre.c_str());
    }

    void SetYear(int32_t year) {
        return m_interface->set_year(m_data, year);
    }

    void SetTrack(int32_t track) {
        return m_interface->set_track(m_data, track);
    }

    CoverFormat DumpCoverArt(std::vector<char>& buf) {
        buf.clear();
        char* data = nullptr;
        uint32_t length = 0;
        auto fmt = m_interface->dump_cover_art(m_data, &data, &length);
        if (data) {
            buf.resize(length);
            memcpy(buf.data(), data, length);
            delete[] data;
        }
        return fmt;
    }

    bool StoreCoverArt(CoverFormat fmt, const char* data, size_t length) {
        return m_interface->store_cover_art(m_data, fmt, data, length);
    }

    bool HasAudioProperties() const {
        return m_interface->has_audio_properties(m_data);
    }

    int32_t Duration() const {
        return m_interface->get_duration(m_data);
    }

    int32_t BitRate() const {
        return m_interface->get_bit_rate(m_data);
    }

    std::vector<std::string> FileSuffix() const {
        std::vector<std::string> suffixes;
        const char** p = m_interface->get_suffixes(m_data);
        if (p) {
            for (; *p; ++p) {
                suffixes.emplace_back(*p);
            }
        }
        return suffixes;
    }
};

}
