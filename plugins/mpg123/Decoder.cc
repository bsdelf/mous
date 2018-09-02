#include <unistd.h> // for off_t
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <mpg123.h>

#include <plugin/DecoderProto.h>
using namespace mous;

namespace {
    struct Self {
        mpg123_handle* mpg123;

        uint32_t max_bytes_per_unit;
        uint64_t unit_index;
        uint64_t unit_count;

        int32_t channels;
        int32_t bits_per_sample;
        int32_t sample_rate;
        int32_t bit_rate;
        uint64_t duration;

        Self(mpg123_handle* mpg123, uint32_t max_bytes_per_unit)
            : mpg123(mpg123)
            , max_bytes_per_unit(max_bytes_per_unit) {

        }
    };
}

static void* Create() {
    int error = mpg123_init();
    auto decoders = mpg123_supported_decoders();
    assert(decoders && decoders[0]);
    auto mpg123 = mpg123_parnew(nullptr, decoders[0], &error);
    mpg123_param(mpg123, MPG123_FLAGS, MPG123_QUIET | MPG123_SKIP_ID3V2, 0);
    auto max_bytes_per_unit = mpg123_safe_buffer();
    return new Self(mpg123, max_bytes_per_unit);
}

static void Destroy(void* ptr) {
    if (SELF->mpg123) {
        mpg123_close(SELF->mpg123);
        mpg123_delete(SELF->mpg123);
        SELF->mpg123 = nullptr;
    }
    mpg123_exit();
    delete SELF;
}

static ErrorCode Open(void* ptr, const char* url) {
    if (!SELF->mpg123) {
        return ErrorCode::DecoderFailedToInit;
    }

    if (mpg123_open(SELF->mpg123, url) != MPG123_OK) {
        return ErrorCode::DecoderFailedToOpen;
    }

    mpg123_scan(SELF->mpg123);
    mpg123_seek_frame(SELF->mpg123, 0, SEEK_END);
    SELF->unit_count = mpg123_tellframe(SELF->mpg123);
    mpg123_seek_frame(SELF->mpg123, 0, SEEK_SET);

    if (SELF->unit_count <= 0) {
        return ErrorCode::DecoderFailedToOpen;
    }

    long sampleRate;
    int channels;
    int encoding;
    mpg123_getformat(SELF->mpg123, &sampleRate, &channels, &encoding);

    SELF->channels = channels;
    SELF->sample_rate = sampleRate;
    switch (encoding) {
        case MPG123_ENC_SIGNED_16:
        case MPG123_ENC_UNSIGNED_16:
        case MPG123_ENC_16:
            SELF->bits_per_sample = 16;
            break;
        default:
            SELF->bits_per_sample = 8;
    }

    SELF->duration = mpg123_tpf(SELF->mpg123) * 1000.f * SELF->unit_count;
    SELF->unit_index = 0;

    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (SELF->mpg123 != nullptr) {
        mpg123_close(SELF->mpg123);
    }
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    if (SELF->unit_index >= SELF->unit_count) {
        *used = 0;
        return ErrorCode::DecoderOutOfRange;
    }

    mpg123_frameinfo info;
    mpg123_info(SELF->mpg123, &info);
    SELF->bit_rate = info.bitrate;

    mpg123_replace_buffer(SELF->mpg123, reinterpret_cast<unsigned char*>(data), SELF->max_bytes_per_unit);
    off_t num = SELF->unit_index;
    size_t bytes = 0;
    mpg123_decode_frame(SELF->mpg123, &num, nullptr, &bytes);
    SELF->unit_index = num;
    *used = bytes;
    *unit_count = 1;
    ++SELF->unit_index;

    return ErrorCode::Ok;
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    if (index >= SELF->unit_count) {
        return ErrorCode::DecoderOutOfRange;
    }
    SELF->unit_index = index;
    mpg123_seek_frame(SELF->mpg123, SELF->unit_index, SEEK_SET);
    return ErrorCode::Ok;
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    return SELF->max_bytes_per_unit;
}

static uint64_t GetUnitIndex(void* ptr) {
    return SELF->unit_index;
}

static uint64_t GetUnitCount(void* ptr) {
    return SELF->unit_count;
}

static AudioMode GetAudioMode(void* ptr) {
    return AudioMode::Stereo;
}

static int32_t GetChannels(void* ptr) {
    return SELF->channels;
}

static int32_t GetBitsPerSample(void* ptr){
    return SELF->bits_per_sample;
}

static int32_t GetSampleRate(void* ptr) {
    return SELF->sample_rate;
}

static int32_t GetBitRate(void* ptr) {
    return SELF->bit_rate; 
}

static uint64_t GetDuration(void* ptr) {
    return SELF->duration;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

static const char** GetSuffixes(void* ptr) {
    (void) ptr;
    static const char* suffixes[] { "mp3", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "mp3", nullptr };
    return encodings;
}