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

ErrorCode OssRenderer::OpenDevice(const std::string& path)
{
    m_Fd = open(path.c_str(), O_WRONLY);
    m_IsOpened = (m_Fd == -1) ? false : true;
    return (m_Fd >= 0 && m_IsOpened) ? MousOk : MousRendererFailedToOpen;
}

void OssRenderer::CloseDevice()
{
    if (!m_IsOpened || m_Fd == -1)
	return;

    ioctl(m_Fd, SNDCTL_DSP_SYNC);
    close(m_Fd);

    m_Fd = -1;
    m_IsOpened = false;
}

ErrorCode OssRenderer::SetupDevice(uint32_t channels, uint32_t sampleRate, uint32_t bitsPerSample)
{
    if (!m_IsOpened)
	return MousRendererFailedToSetup;

    int err = 0;
    int _channels = channels;
    int _sampleRate = sampleRate;
    int _bitsPerSample = bitsPerSample;

    err = ioctl(m_Fd, SOUND_PCM_WRITE_CHANNELS, &_channels);

    if (err < 0 || _channels != channels)
	return MousRendererBadChannels;

    err = ioctl(m_Fd, SOUND_PCM_WRITE_RATE, &_sampleRate);
    if (err < 0 || _sampleRate != sampleRate)
	return MousRendererBadSampleRate;

    err = ioctl(m_Fd, SOUND_PCM_WRITE_BITS, &_bitsPerSample);
    if (err < 0 || _bitsPerSample != bitsPerSample)
	return MousRendererBadBitsPerSample;

    return MousOk;
}

ErrorCode OssRenderer::WriteDevice(const char* buf, uint32_t len)
{
    for (int off = 0, nw = 0, left = len; left >= 0; left -=nw, off += nw) {
	nw = write(m_Fd, buf+off, left);
	if (nw <= 0)
	    return MousRendererFailedToWrite;
    }
    return MousOk;
}
