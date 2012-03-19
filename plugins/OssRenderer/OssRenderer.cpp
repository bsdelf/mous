#include "OssRenderer.h"
#include <fcntl.h>  // open
#include <unistd.h> // write
#include <sys/ioctl.h>
#include <sys/soundcard.h>

OssRenderer::OssRenderer():
    m_Fd(-1),
    m_IsOpened(false)
{

}

OssRenderer::~OssRenderer()
{
    CloseDevice();
}

EmErrorCode OssRenderer::OpenDevice(const std::string& path)
{
    if (m_PrevPath != path)
        m_PrevPath = path;
    m_Fd = open(m_PrevPath.c_str(), O_WRONLY);
    m_IsOpened = (m_Fd < 0) ? false : true;
    return (m_Fd >= 0 && m_IsOpened) ? ErrorCode::Ok : ErrorCode::RendererFailedToOpen;
}

void OssRenderer::CloseDevice()
{
    if (!m_IsOpened || m_Fd == -1)
        return;

    //ioctl(m_Fd, SNDCTL_DSP_SYNC);
    close(m_Fd);

    m_Fd = -1;
    m_IsOpened = false;
}

EmErrorCode OssRenderer::SetupDevice(int32_t channels, int32_t sampleRate, int32_t bitsPerSample)
{
    if (m_IsOpened &&
            (channels != m_Channels || 
             sampleRate != m_SampleRate || 
             bitsPerSample != m_BitsPerSample)) {
        CloseDevice();
        EmErrorCode ret = OpenDevice(m_PrevPath);
        if (ret != ErrorCode::Ok)
            return ret;
    }

    int err = 0;
    int _channels = channels;
    int _sampleRate = sampleRate;
    int _bitsPerSample = bitsPerSample;

    err = ioctl(m_Fd, SNDCTL_DSP_CHANNELS, &_channels);
    if (err < 0 || _channels != channels)
        return ErrorCode::RendererBadChannels;

    err = ioctl(m_Fd, SNDCTL_DSP_SPEED, &_sampleRate);
    if (err < 0 || _sampleRate != sampleRate)
        return ErrorCode::RendererBadSampleRate;

    err = ioctl(m_Fd, SNDCTL_DSP_SETFMT, &_bitsPerSample);
    if (err < 0 || _bitsPerSample != bitsPerSample)
        return ErrorCode::RendererBadBitsPerSample;

    m_Channels = channels;
    m_SampleRate = sampleRate;
    m_BitsPerSample = bitsPerSample;

    return ErrorCode::Ok;
}

EmErrorCode OssRenderer::WriteDevice(const char* buf, uint32_t len)
{
    for (int off = 0, nw = 0, left = len; left > 0; left -= nw, off += nw) {
        nw = write(m_Fd, buf+off, left);
        if (nw < 0)
            return ErrorCode::RendererFailedToWrite;
    }
    return ErrorCode::Ok;
}

int OssRenderer::GetVolumeLevel() const
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
