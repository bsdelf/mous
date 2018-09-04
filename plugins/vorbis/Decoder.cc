#include <string.h>
#include <vorbis/vorbisfile.h>

#include <plugin/DecoderProto.h>
using namespace mous;

namespace {
    struct Self {
        OggVorbis_File file;
        int bitstream;
        uint64_t unit_index;
        uint64_t unit_count;
        int32_t channels;
        int32_t bits_per_sample;
        int32_t sample_rate;
        int32_t bit_rate;
        uint64_t duration;
    };
}

static void* Create() {
    return new Self();
}

static void Destroy(void* ptr) {
    delete SELF;
}

static ErrorCode Open(void* ptr, const char* url) {
    if (ov_fopen(url, &SELF->file) != 0) {
        return ErrorCode::DecoderFailedToOpen;
    }
    auto vi = ov_info(&SELF->file, -1);
    if (!vi) {
        return ErrorCode::DecoderFailedToInit;
    }
    SELF->bitstream = 0;
    SELF->unit_index = 0;
    SELF->unit_count = ov_pcm_total(&SELF->file, -1);
    SELF->duration = ov_time_total(&SELF->file, -1) * 1000;
    SELF->channels = vi->channels;
    SELF->sample_rate = vi->rate;
    SELF->bits_per_sample = 16;
    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    ov_clear(&SELF->file);
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    if (SELF->unit_index >= SELF->unit_count) {
        return ErrorCode::DecoderOutOfRange;
    }
    const auto bytes = ov_read(&SELF->file, data, GetMaxBytesPerUnit(ptr), 0, 2, 1, &SELF->bitstream);
    if (bytes <= 0) {
        return ErrorCode::DecoderOutOfRange;
    }
    *used = bytes;
    *unit_count = ov_pcm_tell(&SELF->file) - SELF->unit_index;
    SELF->unit_index += *unit_count;
    SELF->bit_rate = ov_info(&SELF->file, SELF->bitstream)->bitrate_nominal / 1000;
    return ErrorCode::Ok;
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    if (index >= SELF->unit_count) {
        return ErrorCode::DecoderOutOfRange;
    }
    const auto ok = ov_pcm_seek(&SELF->file, index);
    if (ok != 0) {
        return ErrorCode::DecoderOutOfRange;
    }
    SELF->unit_index = index;
    return ErrorCode::Ok;
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    return 4096;
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

static int32_t GetBitsPerSample(void* ptr) {
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
    static const char* suffixes[] { "ogg", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "vorbis", nullptr };
    return encodings;
}