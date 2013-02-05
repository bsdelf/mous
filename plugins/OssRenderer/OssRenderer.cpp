#include "OssRenderer.h"
#include <fcntl.h>  // open
#include <unistd.h> // write
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <errno.h>
#include <cstring>
#include <iostream>
using namespace std;

OssRenderer::OssRenderer():
    m_Fd(-1),
    m_IsOpened(false),
    m_Channels(-1),
    m_SampleRate(-1),
    m_BitsPerSample(-1)
{
    m_OptDevicePath.desc = "Output device.";
    m_OptDevicePath.userVal = m_OptDevicePath.defaultVal = "/dev/dsp";
}

OssRenderer::~OssRenderer()
{
    Close();
}

EmErrorCode OssRenderer::Open()
{
    if (m_PrevPath != m_OptDevicePath.userVal)
        m_PrevPath = m_OptDevicePath.userVal;
    m_Fd = open(m_PrevPath.c_str(), O_WRONLY);
    m_IsOpened = (m_Fd < 0) ? false : true;
    return (m_Fd >= 0 && m_IsOpened) ? ErrorCode::Ok : ErrorCode::RendererFailedToOpen;
}

void OssRenderer::Close()
{
    if (!m_IsOpened || m_Fd < 0)
        return;

    ioctl(m_Fd, SNDCTL_DSP_SYNC);
    close(m_Fd);

    m_Fd = -1;
    m_IsOpened = false;
}

EmErrorCode OssRenderer::Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
{
    if (m_IsOpened
            && channels == m_Channels
            && sampleRate == m_SampleRate
            && bitsPerSample == m_BitsPerSample) {
        return ErrorCode::Ok;
    }

    Close();
    EmErrorCode ret = Open();
    if (ret != ErrorCode::Ok)
        return ret;

    int err = 0;
    int _channels = channels;
    int _sampleRate = sampleRate;
    int _bitsPerSample = bitsPerSample;

    err = ioctl(m_Fd, SNDCTL_DSP_CHANNELS, &_channels);
    errno = 0;
    if (err == -1 || _channels != channels) {
        channels = _channels;
        cout << strerror(errno) << endl;
        return ErrorCode::RendererBadChannels;
    }

    errno = 0;
    err = ioctl(m_Fd, SNDCTL_DSP_SPEED, &_sampleRate);
    if (err == -1 || _sampleRate != sampleRate) {
        sampleRate = _sampleRate;
        cout << strerror(errno) << endl;
        return ErrorCode::RendererBadSampleRate;
    }

    errno = 0;
    err = ioctl(m_Fd, SNDCTL_DSP_SETFMT, &_bitsPerSample);
    if (err == -1 || _bitsPerSample != bitsPerSample) {
        bitsPerSample = _bitsPerSample;
        cout << strerror(errno) << endl;
        return ErrorCode::RendererBadBitsPerSample;
    }

    m_Channels = channels;
    m_SampleRate = sampleRate;
    m_BitsPerSample = bitsPerSample;

    return ErrorCode::Ok;
}

EmErrorCode OssRenderer::Write(const char* buf, uint32_t len)
{
    for (int off = 0, nw = 0, left = len; left > 0; left -= nw, off += nw) {
        nw = write(m_Fd, buf+off, left);
        if (nw < 0)
            return ErrorCode::RendererFailedToWrite;
    }
    return ErrorCode::Ok;
}

#ifndef SNDCTL_DSP_GETPLAYVOL 
#define SNDCTL_DSP_GETPLAYVOL MIXER_READ(SOUND_MIXER_VOLUME)
#endif

#ifndef SNDCTL_DSP_SETPLAYVOL
#define SNDCTL_DSP_SETPLAYVOL MIXER_WRITE(SOUND_MIXER_VOLUME)
#endif

int OssRenderer::VolumeLevel() const
{
    int level = 0;
    ioctl(m_Fd, SNDCTL_DSP_GETPLAYVOL, &level);
    level = ((level >> 8) + (level & 0xff)) / 2;
    return level;
}

void OssRenderer::SetVolumeLevel(int level)
{
    int all = (level) | (level << 8);
    ioctl(m_Fd, SNDCTL_DSP_SETPLAYVOL, &all);
}

std::vector<const BaseOption*> OssRenderer::Options() const
{
    std::vector<const BaseOption*> list(1);
    list[0] = &m_OptDevicePath;
    return list;
}
