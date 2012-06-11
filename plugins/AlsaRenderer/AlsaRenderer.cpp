#include "AlsaRenderer.h"
#include <algorithm>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

AlsaRenderer::AlsaRenderer():
    m_DeviceName("default"),
    m_PcmHandle(NULL),
    m_Dir(0),
    m_Resample(1),
    m_FrameLength(0),
    m_BitsPerSample(0)
{
}

AlsaRenderer::~AlsaRenderer()
{
    Close();
}

EmErrorCode AlsaRenderer::Open()
{
    int ret = snd_pcm_open(&m_PcmHandle, m_DeviceName.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (ret != 0)
        return ErrorCode::RendererFailedToOpen;

    return ErrorCode::Ok;
}

void AlsaRenderer::Close()
{
    if (m_PcmHandle != NULL) {
        snd_pcm_drain(m_PcmHandle);
        snd_pcm_close(m_PcmHandle);
        m_PcmHandle = NULL;
    }
}

EmErrorCode AlsaRenderer::Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
{
    m_Channels.val = channels;
    m_SampleRate.val = sampleRate;
    m_BitsPerSample = bitsPerSample;

    bool ok = SetupHwParams();
    if (ok)
        SetupSwParams();

    if ((unsigned int)channels != m_Channels.val) {
        channels = m_Channels.val;
        ok = false;
    }
    if ((unsigned int)sampleRate != m_SampleRate.val) {
        sampleRate = m_SampleRate.val;
        //NOTE: just is a workaround :-(
        ok = sampleRate > 1 ? true : false;
    }
    if ((unsigned int)bitsPerSample != m_BitsPerSample) {
        bitsPerSample = m_BitsPerSample;
        ok = false;
    }

    return ok ? ErrorCode::Ok : ErrorCode::RendererFailedToSetup;
}

EmErrorCode AlsaRenderer::Write(const char* buf, uint32_t len)
{
    int off = 0;
    int leftFrames = (len + m_FrameLength - 1) / m_FrameLength;

    while (leftFrames > 0) {
        int written = snd_pcm_writei(m_PcmHandle, buf+off, leftFrames);
        if (written > 0) {
            leftFrames -= written;
            off += snd_pcm_frames_to_bytes(m_PcmHandle, written);
        } else if (written == -EPIPE) {
            if (snd_pcm_prepare(m_PcmHandle) < 0)
                printf("FATAL: snd_pcm_prepare() failed!\n");
        } else if (written == -ESTRPIPE) {
            while ((written = snd_pcm_resume(m_PcmHandle)) == -EAGAIN)
                usleep(100);
            if (written < 0) {
                if ((written = snd_pcm_prepare(m_PcmHandle)) < 0)
                    return ErrorCode::RendererFailedToWrite;
            }
        } else if (written <= 0) {
            printf("writei error / short write\n");
        }
    }

    return ErrorCode::Ok;
}

int AlsaRenderer::VolumeLevel() const
{
    return 0;
}

void AlsaRenderer::SetVolumeLevel(int level)
{
}

bool AlsaRenderer::Options(vector<const BaseOption*>& list) const
{
    return false;
}

bool AlsaRenderer::SetupHwParams()
{
    snd_pcm_hw_params_t* params;

    /* allocate a hardware parameters object */
    snd_pcm_hw_params_malloc(&params);

    /* choose all parameters */
    snd_pcm_hw_params_any(m_PcmHandle, params);

    /* enable hardware resampling */
    snd_pcm_hw_params_set_rate_resample(m_PcmHandle, params, m_Resample);

    /* set the interleaved read/write format */
    snd_pcm_hw_params_set_access(m_PcmHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    /* signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(m_PcmHandle, params, SND_PCM_FORMAT_S16_LE);

    /* set the count of channels */
    snd_pcm_hw_params_get_channels_max(params, &m_Channels.max);
    snd_pcm_hw_params_get_channels_min(params, &m_Channels.min);
    if (m_Channels.val < m_Channels.min || m_Channels.val > m_Channels.max) 
        m_Channels.val = m_Channels.min;
    snd_pcm_hw_params_set_channels(m_PcmHandle, params, m_Channels.val);

    /* set the stream rate */
    snd_pcm_hw_params_get_rate_max(params, &m_SampleRate.max, &m_Dir);
    snd_pcm_hw_params_get_rate_min(params, &m_SampleRate.min, &m_Dir);
    if (m_SampleRate.val < m_SampleRate.min || m_SampleRate.val > m_SampleRate.max)
        m_SampleRate.val = m_SampleRate.min;
    printf("sample rate max:%d, min:%d, val:%d\n", 
            m_SampleRate.max, m_SampleRate.min, m_SampleRate.val);
    snd_pcm_hw_params_set_rate(m_PcmHandle, params, m_SampleRate.val, m_Dir);

    /* we can set period and buffer by size or time */
    /* by default we use "size" (count of frames)   */

    /* set how many frames in a buffer (a buffer cantains several periods) */
    snd_pcm_hw_params_get_buffer_size_max(params, &m_BufferSize.max);
    snd_pcm_hw_params_get_buffer_size_min(params, &m_BufferSize.min);

    /* detect period time and buffer time range  */
    snd_pcm_hw_params_get_buffer_time_max(params, &m_BufferTime.max, &m_Dir);
    snd_pcm_hw_params_get_buffer_time_min(params, &m_BufferTime.min, &m_Dir);
    m_BufferTime.val = std::max(m_BufferTime.max / 2, m_BufferTime.min); 
    //m_BufferTime.val = m_BufferTime.min + (m_BufferTime.max - m_BufferTime.min) /2; 
    printf("buffer time max:%d, min:%d, val:%d\n", 
            m_BufferTime.max, m_BufferTime.min, m_BufferTime.val);
    //(m_BufferTime.max > 120000) ? 120000 : m_BufferTime.max;//120000
    snd_pcm_hw_params_set_buffer_time_near(m_PcmHandle, params, &m_BufferTime.val, &m_Dir);

    snd_pcm_hw_params_get_period_time_max(params, &m_PeriodTime.max, &m_Dir);
    snd_pcm_hw_params_get_period_time_min(params, &m_PeriodTime.min, &m_Dir);
    m_PeriodTime.val = m_BufferTime.val / 4;
    //m_PeriodTime.val = m_PeriodTime.min + (m_PeriodTime.max - m_PeriodTime.min) / 2;
    printf("period time max:%d, min:%d, val:%d\n", 
            m_PeriodTime.max, m_PeriodTime.min, m_PeriodTime.val);
    snd_pcm_hw_params_set_period_time_near(m_PcmHandle, params, &m_PeriodTime.val, &m_Dir);

    int ret = snd_pcm_hw_params(m_PcmHandle, params);

    if (ret != 0) {
        snd_pcm_hw_params_free(params);
        return false;
    }

    // get period size again
    snd_pcm_hw_params_get_period_size(params, &m_PeriodSize.val, &m_Dir);
    snd_pcm_hw_params_get_buffer_size(params, &m_BufferSize.val);

    m_BitsPerSample = snd_pcm_format_physical_width(SND_PCM_FORMAT_S16_LE);
    m_FrameLength = (m_BitsPerSample/8 * m_Channels.val);
    //mPeriodBufferLength = m_PeriodSize.val * m_FrameLength ;

    snd_pcm_hw_params_free(params);

    return true;
}

void AlsaRenderer::SetupSwParams()
{
    snd_pcm_sw_params_t* params;

    /* allocate a software parameters object */
    snd_pcm_sw_params_malloc(&params);

    /* get the current swparams */
    snd_pcm_sw_params_current(m_PcmHandle, params);

    /* round up to closest transfer boundary */
    snd_pcm_sw_params_set_start_threshold(m_PcmHandle, params, 1);

    /* require a minimum of one full transfer in the buffer */
    snd_pcm_sw_params_set_avail_min(m_PcmHandle, params, 1);

    snd_pcm_sw_params(m_PcmHandle, params);

    /* free */
    snd_pcm_sw_params_free(params);
}
