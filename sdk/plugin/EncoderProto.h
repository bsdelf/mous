#pragma once

#include <unistd.h>
#include <inttypes.h>
#include <util/AudioMode.h>
#include <util/ErrorCode.h>
#include <util/Option.h>
#include "Interface.h"

static void* Create();
static void Destroy(void* ptr);
static void SetChannels(void* ptr, int32_t channels);
static void SetSampleRate(void* ptr, int32_t sample_rate);
static void SetBitsPerSample(void* ptr, int32_t bits_per_sample);
static void SetMediaTag(void* ptr, const mous::MediaTag* tag);
static mous::ErrorCode OpenOutput(void* ptr, const char* path);
static void CloseOutput(void* ptr);
static mous::ErrorCode Encode(void* ptr, const char* data, uint32_t length);
static mous::ErrorCode Flush(void* ptr);
static const mous::BaseOption** GetOptions(void* ptr);
static const char* GetSuffix(void* ptr);

static mous::EncoderInterface encoder_interface {
    Create,
    Destroy,
    SetChannels,
    SetSampleRate,
    SetBitsPerSample,
    SetMediaTag,
    OpenOutput,
    CloseOutput,
    Encode,
    Flush,
    GetOptions,
    GetSuffix
};

extern "C" {
    const mous::EncoderInterface* MousGetEncoderInterface() {
        return &encoder_interface;
    }
}