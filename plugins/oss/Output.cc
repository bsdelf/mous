#include <errno.h>
#include <fcntl.h>  // open
#include <unistd.h> // write
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>

using namespace std;

#include <plugin/OutputProto.h>
using namespace mous;

namespace {
    struct Self {
        std::string prev_path;
        int fd = -1;
        bool is_opened = false;
        int32_t channels = -1;
        int32_t sample_rate = -1;
        int32_t bits_per_sample = -1;
        StringOption option_device_path;
        std::vector<const BaseOption*> options;

        Self() {
            options = {
                &option_device_path,
                nullptr;
            };
        }
    };
}

static void* Create() {
    auto self = new Self;
    self->option_device_path.desc = "Output device.";
    self->option_device_path.userVal = self->option_device_path.defaultVal = "/dev/dsp";
    return self;
}

static void Destroy(void* ptr) {
    Close(ptr);
    delete SELF;
}

static ErrorCode Open(void* ptr) {
    if (SELF->prev_path != SELF->option_device_path.userVal) {
        SELF->prev_path = SELF->option_device_path.userVal;
    }
    SELF->fd = ::open(SELF->prev_path.c_str(), O_WRONLY);
    SELF->is_opened = (SELF->fd < 0) ? false : true;
    return (SELF->fd >= 0 && SELF->is_opened) ? ErrorCode::Ok : ErrorCode::OutputFailedToOpen;
}

static void Close(void* ptr) {
    if (!SELF->is_opened || SELF->fd < 0)
        return;

    ::ioctl(SELF->fd, SNDCTL_DSP_SYNC);
    ::close(SELF->fd);

    SELF->fd = -1;
    SELF->is_opened = false;
}

static ErrorCode Setup(void* ptr, int32_t* channels, int32_t* sample_rate, int32_t* bits_per_sample) {
    if (SELF->is_opened
            && *channels == SELF->channels
            && *sample_rate == SELF->sample_rate
            && *bits_per_sample == SELF->bits_per_sample) {
        return ErrorCode::Ok;
    }

    Close(ptr);
    ErrorCode ret = Open(ptr);
    if (ret != ErrorCode::Ok)
        return ret;

    int err = 0;
    int _channels = *channels;
    int _sampleRate = *sample_rate;
    int _bitsPerSample = *bits_per_sample;

    err = ::ioctl(SELF->fd, SNDCTL_DSP_CHANNELS, &_channels);
    errno = 0;
    if (err == -1 || _channels != *channels) {
        *channels = _channels;
        printf("[oss] %s\n", ::strerror(errno));
        return ErrorCode::OutputBadChannels;
    }

    errno = 0;
    err = ::ioctl(SELF->fd, SNDCTL_DSP_SPEED, &_sampleRate);
    if (err == -1 || _sampleRate != *sample_rate) {
        *sample_rate = _sampleRate;
        printf("[oss] %s\n", ::strerror(errno));
        return ErrorCode::OutputBadSampleRate;
    }

    errno = 0;
    err = ::ioctl(SELF->fd, SNDCTL_DSP_SETFMT, &_bitsPerSample);
    if (err == -1 || _bitsPerSample != *bits_per_sample) {
        *bits_per_sample = _bitsPerSample;
        printf("[oss] %s\n", ::strerror(errno));
        return ErrorCode::OutputBadBitsPerSample;
    }

    SELF->channels = *channels;
    SELF->sample_rate = *sample_rate;
    SELF->bits_per_sample = *bits_per_sample;

    return ErrorCode::Ok;
}

static ErrorCode Write(void* ptr, const char* buf, uint32_t len) {
    for (int off = 0, nw = 0, left = len; left > 0; left -= nw, off += nw) {
        nw = ::write(SELF->fd, buf+off, left);
        if (nw < 0) {
            return ErrorCode::OutputFailedToWrite;
        }
    }
    return ErrorCode::Ok;
}

#ifndef SNDCTL_DSP_GETPLAYVOL 
#define SNDCTL_DSP_GETPLAYVOL MIXER_READ(SOUND_MIXER_VOLUME)
#endif

#ifndef SNDCTL_DSP_SETPLAYVOL
#define SNDCTL_DSP_SETPLAYVOL MIXER_WRITE(SOUND_MIXER_VOLUME)
#endif

static int GetVolume(void* ptr) {
    // all=right|left 16bits
    int all = 0;
    ::ioctl(SELF->fd, SNDCTL_DSP_GETPLAYVOL, &all);
    int avg = ((all >> 8) + (all & 0xff)) / 2;
    return avg;
}

static void SetVolume(void* ptr, int avg) {
    int all = (avg) | (avg << 8);
    ::ioctl(SELF->fd, SNDCTL_DSP_SETPLAYVOL, &all);
}

static const BaseOption** GetOptions(void* ptr) {
    return SELF->options.data();
}
