#pragma once

#include <stdio.h>
#include <vorbis/vorbisenc.h>

#include <plugin/EncoderProto.h>
using namespace mous;

struct Self {
    RangedFloatOption quality;
    EnumedIntOption bit_rate;
    GroupedOption vbr_or_abr;

    FILE* file = nullptr;

    int channels = 0;
    int sample_rate = 0;
    int bits_per_sample = 0;

    vorbis_info vorbis_info;

    const MediaTag* media_tag = nullptr;
};

static void* Create() {
    auto self = new Self();

    self->quality.desc = "Quality Level";
    self->quality.min = -1;
    self->quality.max = 10;
    self->quality.defaultVal = 5;
    self->quality.userVal = 5;
    self->quality.point = 10;

    self->bit_rate.desc = "Bit Rate";
    int rates[] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
    self->bit_rate.enumedVal.assign(rates, rates + sizeof(rates)/sizeof(int));
    self->bit_rate.defaultChoice = sizeof(rates)/sizeof(int) - 4;
    self->bit_rate.userChoice = sizeof(rates)/sizeof(int) - 4;

    self->vbr_or_abr.desc = "Encode with";
    self->vbr_or_abr.groups.resize(2);
    self->vbr_or_abr.groups[0].first = "VBR (Recommended)";
    self->vbr_or_abr.groups[0].second.push_back(&self->quality);
    self->vbr_or_abr.groups[1].first = "ABR (Be careful)";
    self->vbr_or_abr.groups[1].second.push_back(&self->bit_rate);
    self->vbr_or_abr.defaultUse = 0;
    self->vbr_or_abr.userUse = 0;

    return self;
}

static void Destroy(void* ptr) {
    CloseOutput(ptr);
    delete SELF;
}

void SetChannels(void* ptr, int32_t channels) {
    SELF->channels = channels;
}

static void SetSampleRate(void* ptr, int32_t sampleRate)
{
    SELF->sample_rate = sampleRate;
}

static void SetBitsPerSample(void* ptr, int32_t bitsPerSample) {
    SELF->bits_per_sample = bitsPerSample;
}

static void SetMediaTag(void* ptr, const MediaTag* tag)
{
    SELF->media_tag = tag;
}

static ErrorCode OpenOutput(void* ptr, const char* path)
{
    SELF->file = ::fopen(path, "wb+");
    if (SELF->file == nullptr) {
        return ErrorCode::EncoderFailedToOpen;
    }

    int ret = 0;
    vorbis_info_init(&SELF->vorbis_info);
    if (SELF->vbr_or_abr.userUse == 0) {
        double point = SELF->quality.point;
        ret = vorbis_encode_init_vbr(&SELF->vorbis_info, SELF->channels, SELF->sample_rate, SELF->quality.userVal/point);
    }

    if (ret != 0) {
        return ErrorCode::EncoderFailedToInit;
    }

    return ErrorCode::Ok;
}

static void CloseOutput(void* ptr)
{
    if (SELF->file != nullptr) {
        ::fclose(SELF->file);
        SELF->file = nullptr;
    }
}

static ErrorCode Encode(void* ptr, const char* data, uint32_t length) {
    return ErrorCode::EncoderFailedToEncode;
}

static ErrorCode Flush(void* ptr) {
    return ErrorCode::EncoderFailedToFlush;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

static const char* GetSuffix(void* ptr) {
    return "ogg";
}
