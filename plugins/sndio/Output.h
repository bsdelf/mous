#pragma once

#include <sndio.h>
#include <stdio.h>

#include <plugin/OutputProto.h>
using namespace mous;

struct Self {
    struct sio_hdl* sio = nullptr;
    unsigned int volume = 0;
    int32_t channels = -1;
    int32_t sample_rate = -1;
    int32_t bits_per_sample = -1;
};

static void* Create(void) {
    return new Self;
}

static void Close(void* ptr) {
    auto& sio = SELF->sio;
    if (sio) {
        sio_stop(sio);
        sio_close(sio);
        sio = nullptr;
    }
}

static void Destroy(void* ptr) {
    Close(ptr);
    delete SELF;
}

static void OnVolumeChanged(void* ptr, unsigned int vol)
{
    SELF->volume = vol;
}

static ErrorCode Open(void* ptr) {
    auto& sio = SELF->sio;
    sio = sio_open(SIO_DEVANY, SIO_PLAY, 0);
    sio_onvol(sio, OnVolumeChanged, ptr);
    return sio ? ErrorCode::Ok : ErrorCode::OutputFailedToOpen;
}

static ErrorCode Setup(void* ptr, int32_t* channels, int32_t* sample_rate, int32_t* bits_per_sample) {
    if (SELF->sio &&
        SELF->channels == *channels &&
        SELF->sample_rate == *sample_rate &&
        SELF->bits_per_sample == *bits_per_sample) {
        return ErrorCode::Ok;
    }

    Close(ptr);
    ErrorCode ret = Open(ptr);
    if (ret != ErrorCode::Ok) {
        return ret;
    }

    struct sio_par par;
    sio_initpar(&par);
    par.bits = *bits_per_sample;
    par.rate = *sample_rate;
    par.pchan = *channels;

    int ok = 0;
    ok = sio_setpar(SELF->sio, &par);
    if (!ok) {
        printf("failed to set\n");
        return ErrorCode::OutputFailedToSetup;
    }
    ok = sio_start(SELF->sio);
    if (!ok) {
        printf("failed to start\n");
        return ErrorCode::OutputFailedToSetup;
    }

    SELF->channels = par.pchan;
    SELF->sample_rate = par.rate;
    SELF->bits_per_sample = par.bits;

    return ErrorCode::Ok;
}

static ErrorCode Write(void* ptr, const char* data, uint32_t length) {
    const auto nbytes = sio_write(SELF->sio, data, length);
    return (nbytes == length) ? ErrorCode::Ok : ErrorCode::OutputFailedToWrite;
}

static int GetVolume(void* ptr) {
    return 100.f * SELF->volume / SIO_MAXVOL;
}

static void SetVolume(void* ptr, int level) {
    const unsigned int vol = level / 100.f * SIO_MAXVOL;
    const int ok = sio_setvol(SELF->sio, vol);
    if (!ok) {
        printf("failed to set volume level\n");
    }
    SELF->volume = vol;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}
