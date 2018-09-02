#pragma once

#include <inttypes.h>
#include <deque>
#include <util/ErrorCode.h>
#include <util/AudioMode.h>
#include <util/CoverFormat.h>
#include <util/MediaItem.h>

#ifndef SELF
#define SELF (static_cast<Self*>(ptr))
#endif

namespace mous {

struct BaseOption;

struct OutputInterface {
    void* (*create)(void);
    void (*destroy)(void* ptr);
    ErrorCode (*open)(void* ptr);
    void (*close)(void* ptr);
    ErrorCode (*setup)(void* ptr, int32_t* channels, int32_t* sample_rate, int32_t* bits_per_sample);
    ErrorCode (*write)(void* ptr, const char* data, uint32_t length);
    int (*get_volume)(void* ptr);
    void (*set_volume)(void* ptr, int level);
    const BaseOption** (*get_options)(void* ptr);
};

struct DecoderInterface {
    void* (*create)(void);
    void (*destroy)(void* ptr);
    ErrorCode (*open)(void* ptr, const char* url);
    void (*close)(void* ptr);
    ErrorCode (*decode_unit)(void* ptr, char* data, uint32_t* used, uint32_t* unit_count);
    ErrorCode (*set_unit_index)(void* ptr, uint64_t index);
    uint32_t (*get_max_bytes_per_unit)(void* ptr);
    uint64_t (*get_unit_index)(void* ptr);
    uint64_t (*get_unit_count)(void* ptr);
    AudioMode (*get_audio_mode)(void* ptr);
    int32_t (*get_channels)(void* ptr);
    int32_t (*get_bits_per_sample)(void* ptr);
    int32_t (*get_sample_rate)(void* ptr);
    int32_t (*get_bit_rate)(void* ptr);
    uint64_t (*get_duration)(void* ptr);
    const BaseOption** (*get_options)(void* ptr);
    const char** (*get_suffixes)(void* ptr);
    const char** (*get_encodings)(void* ptr);
};

struct EncoderInterface {
    void* (*create)(void);
    void (*destroy)(void* ptr);
    void (*set_channels)(void* ptr, int32_t channels);
    void (*set_sample_rate)(void* ptr, int32_t sample_rate);
    void (*set_bits_per_sample)(void* ptr, int32_t bits_per_sample);
    void (*set_media_tag)(void* ptr, const MediaTag* tag);
    ErrorCode (*open_output)(void* ptr, const char* path);
    void (*close_output)(void* ptr);
    ErrorCode (*encode)(void* ptr, const char* data, uint32_t length);
    ErrorCode (*flush)(void* ptr);
    const BaseOption** (*get_options)(void* ptr);
    const char* (*get_suffix)(void* ptr);
};

struct TagParserInterface {
    void* (*create)(void);
    void (*destroy)(void* ptr);
    ErrorCode (*open)(void* ptr, const char* url);
    void (*close)(void* ptr);
    bool (*has_tag)(void* ptr);
    const char* (*get_title)(void* ptr);
    const char* (*get_artist)(void* ptr);
    const char* (*get_album)(void* ptr);
    const char* (*get_comment)(void* ptr);
    const char* (*get_genre)(void* ptr);
    int32_t (*get_year)(void* ptr);
    int32_t (*get_track)(void* ptr);
    bool (*can_edit)(void* ptr);
    bool (*save)(void* ptr);
    void (*set_title)(void* ptr, const char* str);
    void (*set_artist)(void* ptr, const char* str);
    void (*set_album)(void* ptr, const char* str);
    void (*set_comment)(void* ptr, const char* str);
    void (*set_genre)(void* ptr, const char* str);
    void (*set_year)(void* ptr, int32_t num);
    void (*set_track)(void* ptr, int32_t num);
    CoverFormat (*dump_cover_art)(void* ptr, char** out, uint32_t* length);
    bool (*store_cover_art)(void* ptr, CoverFormat fmt, const char* data, size_t length);
    bool (*has_audio_properties)(void* ptr);
    int32_t (*get_duration)(void* ptr);
    int32_t (*get_bit_rate)(void* ptr);
    const mous::BaseOption** (*get_options)(void* ptr);
    const char** (*get_suffixes)(void* ptr);
};

struct SheetParserInterface {
    void* (*create)(void);
    void (*destroy)(void* ptr);
    void (*dump_file)(void* ptr, const char* path, std::deque<MediaItem>* list);
    void (*dump_stream)(void* ptr, const char* stream, std::deque<MediaItem>* list);
    const mous::BaseOption** (*get_options)(void* ptr);
    const char** (*get_suffixes)(void* ptr);
};

struct FormatProbeInterface {
    void* (*create)(void);
    void (*destroy)(void* ptr);
    const char* (*probe)(void* ptr, const char* path);
    const mous::BaseOption** (*get_options)(void* ptr);
    const char** (*get_suffixes)(void* ptr);
};

}
