#include <fstream>
#include <cstring>
using namespace std;

#include <plugin/EncoderProto.h>
using namespace mous;

#include "Common.h"

namespace {
    struct Self {
        std::fstream outfile;
        WavHeader wav_header;
    };
}

static void* Create() {
    auto self = new Self;
    memset(&self->wav_header, 0, sizeof(WavHeader));
    memcpy(self->wav_header.riff_id, "RIFF", 4);
    memcpy(self->wav_header.riff_type, "WAVE", 4);
    memcpy(self->wav_header.format_id, "fmt ", 4);
    memcpy(self->wav_header.data_id, "data", 4);
    self->wav_header.format_tag = 0x0001;
    return self;
}

static void Destroy(void* ptr) {
    delete SELF;
}

void SetChannels(void* ptr, int32_t channels) {
    SELF->wav_header.channels = channels;
}

void SetSampleRate(void* ptr, int32_t sample_rate) {
    SELF->wav_header.sample_rate = sample_rate;
}

static void SetBitsPerSample(void* ptr, int32_t bits_per_sample) {
    SELF->wav_header.bits_per_sample = bits_per_sample;
}

static void SetMediaTag(void* ptr, const MediaTag* tag) {

}

static ErrorCode OpenOutput(void* ptr, const char* path) {
    SELF->outfile.open(path, ios::binary | ios::out);
    if (!SELF->outfile.is_open()) {
        return ErrorCode::EncoderFailedToOpen;
    }
    SELF->outfile.write((char*)&SELF->wav_header, sizeof(WavHeader));
    return ErrorCode::Ok;
}

static void CloseOutput(void* ptr) {
    SELF->wav_header.data_chunk_length = (uint32_t)SELF->outfile.tellg() - (uint32_t)sizeof(WavHeader);
    SELF->wav_header.length_after_riff = SELF->wav_header.data_chunk_length + 36;
    SELF->wav_header.format_chunk_length = 16;//SELF->wav_header.data_chunk_length + 24;
    SELF->wav_header.block_align = SELF->wav_header.channels * ((SELF->wav_header.bits_per_sample + 7) / 8);
    SELF->wav_header.avg_bytes_per_sec = SELF->wav_header.block_align * SELF->wav_header.sample_rate;
    SELF->outfile.seekg(0, ios::beg);
    SELF->outfile.write((char*)&SELF->wav_header, sizeof(WavHeader));

    if (SELF->outfile.is_open()) {
        SELF->outfile.close();
    }
}

static ErrorCode Encode(void* ptr, const char* data, uint32_t length) {
    SELF->outfile.write(data, length);
    return ErrorCode::Ok;
}

static ErrorCode Flush(void* ptr) {
    return ErrorCode::Ok;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

static const char* GetSuffix(void* ptr) {
    return "wav";
}
