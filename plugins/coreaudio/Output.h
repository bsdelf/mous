#pragma once
#include <unistd.h>
#include <iostream>
using namespace std;

#include <plugin/OutputProto.h>
using namespace mous;

extern "C" {
#include "cmus/op.h"
#include "cmus/mixer.h"
}

const char* program_name = "CoreAudioOutput";

#define SELF (static_cast<Self*>(ptr))

struct Self {
    bool closed = true;
};

static void* Create() {
    op_pcm_ops.init();
    op_mixer_ops.init();
    return new Self;
}

static void Destroy(void* ptr) {
    Close(ptr);
    op_pcm_ops.exit();
    op_mixer_ops.exit();
    delete SELF;
}

static ErrorCode Open(void* ptr) {
    int maxVol = 0;
    op_mixer_ops.open(&maxVol);
    SELF->closed = false;
    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (!SELF->closed) {
        op_mixer_ops.close();
        SELF->closed = true;
    }
}

static ErrorCode Setup(void* ptr, int32_t* channels, int32_t* sample_rate, int32_t* bits_per_sample) {
    (void) ptr;
    op_pcm_ops.drop();
    op_pcm_ops.close();
    const sample_format_t sf = sf_bits(*bits_per_sample) | sf_rate(*sample_rate) | sf_channels(*channels) | sf_signed(1);
    op_pcm_ops.open(sf, nullptr);
    return ErrorCode::Ok;
}

static ErrorCode Write(void* ptr, const char* data, uint32_t length) {
    (void) ptr;
    op_pcm_ops.write(data, static_cast<int>(length));
    op_pcm_wait(length);
    return ErrorCode::Ok;
}

static int GetVolume(void* ptr) {
    (void) ptr;
    int l, r;
    op_mixer_ops.get_volume(&l, &r);
    return (l + r) / 2;
}

static void SetVolume(void* ptr, int avg) {
    (void) ptr;
    op_mixer_ops.set_volume(avg, avg);
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}
