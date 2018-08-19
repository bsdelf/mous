#pragma once

#include <lame/lame.h>
#include <stdio.h>

#include <vector>
using namespace std;

#include <plugin/EncoderProto.h>
using namespace mous;

struct Self {
    RangedIntOption quality;
    EnumedIntOption bit_rate;
    BooleanOption replay_gain;

    lame_global_flags* flags = nullptr;
    FILE* file = nullptr;

    int bits_per_sample = 0;

    vector<unsigned char> buffer;

    const MediaTag* media_tag = nullptr;
};

static void* Create() {
    auto self = new Self();

    self->quality.desc = "Quality\n0=best(very slow), 9 worst";
    self->quality.min = 0;
    self->quality.max = 9;
    self->quality.defaultVal = 5;
    self->quality.userVal = 5;

    self->bit_rate.desc = "Bit Rate";
    int rates[] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
    self->bit_rate.enumedVal.assign(rates, rates + sizeof(rates)/sizeof(int));
    self->bit_rate.defaultChoice = sizeof(rates)/sizeof(int) - 4;
    self->bit_rate.userChoice = sizeof(rates)/sizeof(int) - 4;

    self->replay_gain.desc = "ReplayGain";
    self->replay_gain.detail = "Perform ReplayGain Analysis";
    self->replay_gain.defaultChoice = true;
    self->replay_gain.userChoice = true;

    return self;
}

static void Destroy(void* ptr) {
    CloseOutput(ptr);
    delete SELF;
}

static void SetChannels(void* ptr, int32_t channels) {
    ::lame_set_num_channels(SELF->flags, channels);
}

static void SetSampleRate(void* ptr, int32_t sample_rate) {
    ::lame_set_num_samples(SELF->flags, sample_rate);
}

static void SetBitsPerSample(void* ptr, int32_t bits_per_sample) {
    SELF->bits_per_sample = bits_per_sample;
}

static void SetMediaTag(void* ptr, const MediaTag* tag) {
    SELF->media_tag = tag;
}

static ErrorCode OpenOutput(void* ptr, const char* path) {
    SELF->file = ::fopen(path, "wb+");

    if (SELF->file == nullptr) {
        return ErrorCode::EncoderFailedToOpen;
    }

    // init lame
    SELF->flags = ::lame_init();

    ::lame_set_quality(SELF->flags, SELF->quality.userVal);
    ::lame_set_brate(SELF->flags, SELF->bit_rate.enumedVal[SELF->bit_rate.userChoice]);
    ::lame_set_mode(SELF->flags, ::JOINT_STEREO);
    ::lame_set_findReplayGain(SELF->flags, SELF->replay_gain.userChoice ? 1 : 0);
    ::lame_set_asm_optimizations(SELF->flags, MMX, 1);
    ::lame_set_asm_optimizations(SELF->flags, SSE, 1);
    if (SELF->media_tag != nullptr) {
        lame_set_write_id3tag_automatic(SELF->flags, 1);
        id3tag_init(SELF->flags);
        id3tag_v2_only(SELF->flags);
        id3tag_set_title(SELF->flags, SELF->media_tag->title.c_str());
        id3tag_set_artist(SELF->flags, SELF->media_tag->artist.c_str());
        id3tag_set_album(SELF->flags, SELF->media_tag->album.c_str());
        id3tag_set_comment(SELF->flags, SELF->media_tag->comment.c_str());
        id3tag_set_genre(SELF->flags, SELF->media_tag->genre.c_str());
        id3tag_set_year(SELF->flags, std::to_string(SELF->media_tag->year).c_str());
        id3tag_set_track(SELF->flags, std::to_string(SELF->media_tag->track).c_str());
    }
    int ret = ::lame_init_params(SELF->flags);
    if (ret < 0) {
        return ErrorCode::EncoderFailedToInit;
    }

    return ErrorCode::Ok;
}

static void CloseOutput(void* ptr) {
    if (SELF->file != nullptr) {
        ::fclose(SELF->file);
        SELF->file = nullptr;
    }

    if (SELF->flags != nullptr) {
        ::lame_close(SELF->flags);
        SELF->flags = nullptr;
    }
}

static ErrorCode Encode(void* ptr, const char* data, uint32_t length) {
    // prepare buffer
    const int samplesPerChannel = 
        length / ::lame_get_num_channels(SELF->flags) / (SELF->bits_per_sample / 8); 
    const int minsz = 1.25 * samplesPerChannel + 7200;
    if (SELF->buffer.size() < minsz) {
        SELF->buffer.resize(minsz);
    }

    // encode
    const int ret = lame_encode_buffer_interleaved(
        SELF->flags,
        reinterpret_cast<short int*>(const_cast<char*>(data)),
        samplesPerChannel,
        SELF->buffer.data(),
        SELF->buffer.size()
    );

    if (ret >= 0) {
        if ((int)::fwrite(SELF->buffer.data(), 1, ret, SELF->file) == ret) {
            return ErrorCode::Ok;
        }
    }

    return ErrorCode::EncoderFailedToEncode;
}

static ErrorCode Flush(void* ptr)
{
    int ret = lame_encode_flush(SELF->flags, SELF->buffer.data(), SELF->buffer.size());
    if (ret >= 0) {
        if ((int)::fwrite(SELF->buffer.data(), 1, ret, SELF->file) == ret) {
            ::lame_mp3_tags_fid(SELF->flags, SELF->file);
            return ErrorCode::Ok;
        }
    }
    return ErrorCode::EncoderFailedToFlush;
}

static const BaseOption** GetOptions(void* ptr) {
    static const BaseOption* options[] {
        &SELF->quality,
        &SELF->bit_rate,
        &SELF->replay_gain,
        nullptr
    };
    return options;
}

static const char* GetSuffix(void* ptr) {
    return "mp3";
}
