#include <inttypes.h>
#include <fstream>
#include <cstring>
using namespace std;

#include <plugin/DecoderProto.h>
using namespace mous;

#include "Common.h"

#define SAMPLES_PER_BLOCK 200;

namespace {
    struct Self {
        ifstream input_stream;
        WavHeader wav_header;

        size_t raw_data_offset = 0;
        size_t raw_data_length = 0;

        int sample_length = 0;
        int block_length = 0;
        char* block_buffer = nullptr;
        size_t block_index = 0;
        size_t total_blocks = 0;

        uint64_t duration = 0;
    };
}

static void* Create() {
    auto self = new Self;
    memset(&self->wav_header, 0, sizeof(WavHeader));
    return self;
}

static void Destroy(void* ptr) {
    Close(ptr);
    delete SELF;
}

static ErrorCode Open(void* ptr, const char* url) {
    SELF->input_stream.open(url, ios::binary);
    if (!SELF->input_stream.is_open()) {
        return ErrorCode::DecoderFailedToOpen;
    }

    SELF->input_stream.seekg(0, ios::beg);
    SELF->input_stream.read(reinterpret_cast<char*>(&SELF->wav_header), sizeof(WavHeader));
    
    if (memcmp(SELF->wav_header.riff_id, "RIFF", 4) != 0 ||
        memcmp(SELF->wav_header.riff_type, "WAVE", 4) != 0 ||
        memcmp(SELF->wav_header.format_id, "fmt ", 4) != 0) {
        SELF->input_stream.close();
        return ErrorCode::DecoderFailedToInit;
    }

    SELF->raw_data_offset = sizeof(WavHeader);
    SELF->raw_data_length = SELF->wav_header.data_chunk_length;

    SELF->sample_length = SELF->wav_header.channels * SELF->wav_header.bits_per_sample / 8.f;
    SELF->block_length = SELF->sample_length * SAMPLES_PER_BLOCK;
    SELF->block_buffer = new char[SELF->block_length];
    SELF->block_index = 0;
    SELF->total_blocks = (SELF->raw_data_length + SELF->block_length - 1) / SELF->block_length;

    SELF->duration = static_cast<double>(SELF->raw_data_length)
        / (SELF->wav_header.bits_per_sample / 8 * SELF->wav_header.channels) 
        / SELF->wav_header.sample_rate * 1000;

    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    SELF->input_stream.close();
    memset(&SELF->wav_header, 0, sizeof(WavHeader));
    if (SELF->block_buffer) {
        delete[] SELF->block_buffer;
        SELF->block_buffer = nullptr;
    }
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    SELF->input_stream.read(data, SELF->block_length);
    *used = SELF->input_stream.gcount();
    *unit_count = 1;
    return ErrorCode::Ok;
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    if ((size_t)index > SELF->total_blocks) {
        return ErrorCode::DecoderOutOfRange;
    }
    SELF->block_index = index;
    SELF->input_stream.seekg(SELF->raw_data_offset + SELF->block_index*SELF->block_length, ios::beg);
    return ErrorCode::Ok;
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    return SELF->block_length;
}

static uint64_t GetUnitIndex(void* ptr) {
    return SELF->block_index;
}

static uint64_t GetUnitCount(void* ptr) {
    return SELF->total_blocks;
}

static AudioMode GetAudioMode(void* ptr) {
    return SELF->wav_header.channels == 1 ? AudioMode::Mono : AudioMode::Stereo;
}

static int32_t GetChannels(void* ptr) {
    return SELF->wav_header.channels;
}

static int32_t GetBitsPerSample(void* ptr) {
    return SELF->wav_header.bits_per_sample;
}

static int32_t GetSampleRate(void* ptr) {
    return SELF->wav_header.sample_rate;
}

static int32_t GetBitRate(void* ptr) {
    return SELF->wav_header.avg_bytes_per_sec / 1024;
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
    static const char* suffixes[] { "wav", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "lpcm", nullptr };
    return encodings;
}
