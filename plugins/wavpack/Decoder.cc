#include <string.h>
#include <wavpack/wavpack.h>

#include <vector>
using namespace std;

#include <plugin/DecoderProto.h>
using namespace mous;

#define NUM_SAMPLES 32

namespace {
    struct Self {
        WavpackContext* wavpack_context = nullptr;
        
        vector<int32_t> buffer;
        int bytes_per_sample;

        uint64_t unit_index;
        uint64_t unit_count;

        int32_t channels;
        int32_t bits_per_sample;
        int32_t sample_rate;
        int32_t bit_rate;
        uint64_t duration;
    };
}

static void FormatSamples(int bps, unsigned char *dst, int32_t *src, uint32_t samcnt);

static void* Create() {
    return new Self;
}

static void Destroy(void* ptr) {
    Close(ptr);
    delete SELF;
}

static ErrorCode Open(void* ptr, const char* url) {
    SELF->wavpack_context = WavpackOpenFileInput(url, nullptr, 0, 0);
    if (!SELF->wavpack_context) {
        return ErrorCode::DecoderFailedToOpen;
    }

    if (WavpackGetNumSamples(SELF->wavpack_context) == (uint32_t) -1) {
        return ErrorCode::DecoderFailedToInit;
    }

    SELF->duration = 1000.f * WavpackGetNumSamples(SELF->wavpack_context) / WavpackGetSampleRate(SELF->wavpack_context);
    SELF->channels = WavpackGetNumChannels(SELF->wavpack_context);
    SELF->sample_rate = WavpackGetSampleRate(SELF->wavpack_context);
    SELF->bits_per_sample = WavpackGetBitsPerSample(SELF->wavpack_context);
    SELF->bytes_per_sample = WavpackGetBytesPerSample(SELF->wavpack_context);

    // one sample may not be enough to build a full channel
    SELF->unit_count = WavpackGetNumSamples(SELF->wavpack_context)/SELF->channels;
    SELF->unit_index = 0;

    SELF->buffer.resize(10 * SELF->channels * sizeof(int32_t)); // 1~10 full-samples

    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (SELF->wavpack_context) {
        WavpackCloseFile(SELF->wavpack_context);
        SELF->wavpack_context = nullptr;
    }
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    if (SELF->unit_index >= SELF->unit_count) {
        *used = 0;
        *unit_count = SELF->unit_count;
        SELF->bit_rate = 0;
        return ErrorCode::DecoderOutOfRange;
    }
    // according to wavpack's source code cli/wvunpack.c
    const uint32_t nsamples = WavpackUnpackSamples(
        SELF->wavpack_context, 
        static_cast<int32_t*>(&SELF->buffer[0]), 
        NUM_SAMPLES);
    FormatSamples(
        SELF->bytes_per_sample,
        reinterpret_cast<unsigned char*>(data),
        &SELF->buffer[0],
        nsamples * SELF->channels);
    *used = nsamples * SELF->bytes_per_sample * SELF->channels;
    *unit_count = nsamples / SELF->channels;
    SELF->unit_index += *unit_count;
    SELF->bit_rate = WavpackGetInstantBitrate(SELF->wavpack_context) / 1000;
    return ErrorCode::Ok;
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    if (index < SELF->unit_count && WavpackSeekSample(SELF->wavpack_context, index*SELF->channels) == 0) {
        SELF->unit_index = index;
        return ErrorCode::Ok;
    } else {
        return ErrorCode::DecoderOutOfRange;
    }
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    return (NUM_SAMPLES * SELF->channels * SELF->bytes_per_sample);
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
    static const char* suffixes[] { "wv", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "wavpack", nullptr };
    return encodings;
}

static void FormatSamples(int bps, unsigned char* dst, int32_t *src, uint32_t samcnt) {
    int32_t temp;

    switch (bps) {

    case 1:
        while (samcnt--) {
            *dst++ = *src++ + 128;
        }
        break;

    case 2:
        while (samcnt--) {
            *dst++ = (unsigned char) (temp = *src++);
            *dst++ = (unsigned char) (temp >> 8);
        }
        break;

    case 3:
        while (samcnt--) {
            *dst++ = (unsigned char) (temp = *src++);
            *dst++ = (unsigned char) (temp >> 8);
            *dst++ = (unsigned char) (temp >> 16);
        }
        break;

    case 4:
        while (samcnt--) {
            *dst++ = (unsigned char) (temp = *src++);
            *dst++ = (unsigned char) (temp >> 8);
            *dst++ = (unsigned char) (temp >> 16);
            *dst++ = (unsigned char) (temp >> 24);
        }
        break;
    }
}
