#include "OssRenderer.h"
#include <fcntl.h>  // open
#include <unistd.h> // write
#include <sys/ioctl.h>
#include <sys/soundcard.h>

OssRenderer::OssRenderer():
    m_fd(-1),
    m_isOpened(false)
{

}

OssRenderer::~OssRenderer()
{
    CloseDevice();
}

ErrorCode OssRenderer::OpenDevice(const std::string& path)
{
    m_fd = open(path.c_str(), O_WRONLY);
    m_isOpened = (m_fd == -1) ? false : true;
    return MousOk;
}

void OssRenderer::CloseDevice()
{
    if (!m_isOpened || m_fd == -1)
	return;

    ioctl(m_fd, SNDCTL_DSP_SYNC);
    close(m_fd);

    m_fd = -1;
    m_isOpened = false;
}

ErrorCode OssRenderer::SetupDevice(uint32_t channels, uint32_t sampleRate, uint32_t bitsPerSample)
{
    if (!m_isOpened)
	return MousRendererFailedToSetup;

    int err = 0;
    int _channels = channels;
    int _sampleRate = sampleRate;
    int _bitsPerSample = bitsPerSample;

    err = ioctl(m_fd, SOUND_PCM_WRITE_CHANNELS, &_channels);

    if (err < 0 || _channels != channels)
	return MousRendererBadChannels;

    err = ioctl(m_fd, SOUND_PCM_WRITE_RATE, &_sampleRate);
    if (err < 0 || _sampleRate != sampleRate)
	return MousRendererBadSampleRate;

    err = ioctl(m_fd, SOUND_PCM_WRITE_BITS, &_bitsPerSample);
    if (err < 0 || _bitsPerSample != bitsPerSample)
	return MousRendererBadBitsPerSample;

    return MousOk;
}

ErrorCode OssRenderer::WriteDevice(const char* buf, uint32_t len)
{
    for (int off = 0, nw = 0, left = len; left >= 0; left -=nw, off += nw) {
	nw = write(m_fd, buf+off, left);
	if (nw <= 0)
	    return MousRendererFailedToWrite;
    }
    return MousOk;
}
