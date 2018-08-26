#pragma once

#include <inttypes.h>

#pragma pack(push, 1)

struct WavHeader {
    // RIFF chunk
    char riff_id[4];
    uint32_t length_after_riff;
    char riff_type[4];

    // format chunk
    char format_id[4];
    uint32_t format_chunk_length;
    uint16_t format_tag;
    uint16_t channels;
    uint32_t sample_rate;
    uint32_t avg_bytes_per_sec;
    uint16_t block_align;
    uint16_t bits_per_sample;

    // data chunk
    char data_id[4];
    uint32_t data_chunk_length;
};

#pragma pack(pop)