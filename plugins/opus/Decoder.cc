#include <string.h>
#include <opus/opusfile.h>

#include <plugin/DecoderProto.h>
using namespace mous;

namespace {
    struct Self {
        OggOpusFile* opusfile = nullptr;

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
    return new Self;
}

static void Destroy(void* ptr) {
    delete SELF;
}

static ErrorCode Open(void* ptr, const char* url) {
    int error = 0;
    auto opusfile = op_open_file(url, &error);
    if (error != 0) {
        return ErrorCode::DecoderFailedToOpen;
    }
    const auto count = op_link_count(opusfile);
    if (count < 1) {
        return ErrorCode::DecoderFailedToInit;
    }
    const int link = 0;
    SELF->opusfile = opusfile;
    SELF->channels = 2; // op_channel_count(opusfile, link);
    SELF->unit_index = 0;
    SELF->unit_count = op_pcm_total(opusfile, link);
    SELF->bit_rate = op_bitrate(opusfile, link) / 1000;
    SELF->bits_per_sample = 16;
    SELF->sample_rate = 48000;
    SELF->duration = 1000.f * SELF->unit_count / SELF->sample_rate;
    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (SELF->opusfile) {
        op_free(SELF->opusfile);
        SELF->opusfile = nullptr;
    }
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    if (SELF->unit_index >= SELF->unit_count) {
        *used = 0;
        *unit_count = 0;
        return ErrorCode::DecoderOutOfRange;
    }
    // read 120 ms
    const int samples_per_frame = SELF->sample_rate / 1000.f * 120 * SELF->channels;
    const int samples_per_channel = op_read_stereo(
        SELF->opusfile, reinterpret_cast<opus_int16*>(data), samples_per_frame);
    *used = SELF->channels * samples_per_channel * sizeof(opus_int16);
    *unit_count = samples_per_channel;
    SELF->unit_index += samples_per_channel;
    return ErrorCode::Ok;
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    if (index >= SELF->unit_count) {
        return ErrorCode::DecoderOutOfRange;
    }
    const auto ok = op_pcm_seek(SELF->opusfile, index);
    if (ok != 0) {
        printf("opus SetUnitIndex error: %d\n", ok);
        return ErrorCode::DecoderOutOfRange;
    }
    return ErrorCode::Ok;
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    return SELF->sample_rate / 1000.f * 120 * SELF->channels * sizeof(opus_int16);
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
    static const char* suffixes[] { "opus", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "opus", nullptr };
    return encodings;
}