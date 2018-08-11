#pragma once

#include <sndio.h>
#include <stdio.h>

#include <plugin/IOutput.h>
using namespace mous;

class SndioOutput: public IOutput
{
public:
    virtual ~SndioOutput()
    {
        Close();
    }

    virtual ErrorCode Open()
    {
        if (path_ != optDevicePath_.userVal) {
            path_ = optDevicePath_.userVal;
        }
        handle_ = sio_open(SIO_DEVANY, SIO_PLAY, 0);
        sio_onvol(handle_, SndioOutput::OnVolumeChanged, this);
        return handle_ ? ErrorCode::Ok : ErrorCode::OutputFailedToOpen;
    }

    virtual void Close()
    {
        if (handle_) {
            sio_stop(handle_);
            sio_close(handle_);
            handle_ = nullptr;
        }
    }

    virtual ErrorCode Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
    {
        if (handle_
                && channels == channels_
                && sampleRate == sampleRate_
                && bitsPerSample == bitsPerSample_) {
            return ErrorCode::Ok;
        }

        Close();
        ErrorCode ret = Open();
        if (ret != ErrorCode::Ok) {
            return ret;
        }

        int ok;
        struct sio_par par;

        sio_initpar(&par);
        par.bits = bitsPerSample;
        par.rate = sampleRate;
        par.pchan = channels;

        ok = sio_setpar(handle_, &par);
        if (not ok) {
            printf("failed to set\n");
            return ErrorCode::OutputFailedToSetup;
        }

        ok = sio_start(handle_);
        if (not ok) {
            printf("failed to start\n");
            return ErrorCode::OutputFailedToSetup;
        }

        channels_ = par.pchan;
        sampleRate_ = par.rate;
        bitsPerSample_ = par.bits;

        return ErrorCode::Ok;
    }

    virtual ErrorCode Write(const char* buf, uint32_t len)
    {
        auto nbytes = sio_write(handle_, buf, len);
        if (nbytes != len) {
            return ErrorCode::OutputFailedToWrite;
        }
        return ErrorCode::Ok;
    }

    virtual int VolumeLevel() const
    {
        return 100.f * volume_ / SIO_MAXVOL;
    }

    virtual void SetVolumeLevel(int level)
    {
        unsigned int vol = level / 100.f * SIO_MAXVOL;
        auto ok = sio_setvol(handle_, vol);
        if (not ok) {
            printf("failed to set vol\n");
        }
        volume_ = vol;
    }

    virtual std::vector<const BaseOption*> Options() const
    {
        return {};
    }

    static void OnVolumeChanged(void* arg, unsigned int vol)
    {
        auto self = static_cast<SndioOutput*>(arg);
        self->volume_ = vol;
    }

private:
    std::string path_;
    struct sio_hdl* handle_ = nullptr;
    unsigned int volume_ = 0;
    int32_t channels_ = -1;
    int32_t sampleRate_ = -1;
    int32_t bitsPerSample_ = -1;
    StringOption optDevicePath_;
};

