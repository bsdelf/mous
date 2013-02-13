#include "WvDecoder.h"
#include <string.h>

static void format_samples(int bps, unsigned char *dst, int32_t *src, uint32_t samcnt);

WvDecoder::WvDecoder():
    m_Ctx(nullptr)
{
}

WvDecoder::~WvDecoder()
{
}

vector<string> WvDecoder::FileSuffix() const
{
    vector<string> list;
    list.push_back("wv");
    return list;
}

EmErrorCode WvDecoder::Open(const std::string& url)
{
    m_Ctx = WavpackOpenFileInput(url.c_str(), nullptr, 0, 0);
    if (m_Ctx == nullptr)
        return ErrorCode::DecoderFailedToOpen;

    if (WavpackGetNumSamples(m_Ctx) == (uint32_t) -1)
        return ErrorCode::DecoderFailedToInit;

    m_Duration = (double)WavpackGetNumSamples(m_Ctx) / WavpackGetSampleRate(m_Ctx) * 1000;
    m_Channels = WavpackGetNumChannels(m_Ctx);
    m_SampleRate = WavpackGetSampleRate(m_Ctx);
    m_BitsPerSample = WavpackGetBitsPerSample(m_Ctx);
    m_BytesPerSample = WavpackGetBytesPerSample(m_Ctx);

    // one sample may not be enough to build a full channel
    m_UnitCount = WavpackGetNumSamples(m_Ctx)/m_Channels;
    m_UnitIndex = 0;

    m_Buf.resize(10 * m_Channels * sizeof(int32_t)); // 1~10 full-samples

    return ErrorCode::Ok;
}

void WvDecoder::Close()
{
    WavpackCloseFile(m_Ctx);
}

bool WvDecoder::IsFormatVaild() const
{
    return true;
}

EmErrorCode WvDecoder::DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    if (m_UnitIndex < m_UnitCount) {
        unitCount = 1;

        // according to wavpack's source code cli/wvunpack.c
        uint32_t nsamples = WavpackUnpackSamples(m_Ctx, (int32_t*)&m_Buf[0], unitCount*m_Channels);

        format_samples(m_BytesPerSample, (unsigned char*)data, &m_Buf[0], nsamples*m_Channels);
        used = nsamples * m_BytesPerSample * m_Channels;

        m_UnitIndex += unitCount;
        m_BitRate = WavpackGetInstantBitrate(m_Ctx)/1000;

        return ErrorCode::Ok;
    }

    used = 0;
    unitCount = m_UnitCount;

    m_BitRate = 0;

    return ErrorCode::DecoderOutOfRange;
}

EmErrorCode WvDecoder::SetUnitIndex(uint64_t index)
{
    if (index < m_UnitCount && WavpackSeekSample(m_Ctx, index*m_Channels) == 0) {
        m_UnitIndex = index;
        return ErrorCode::Ok;
    } else {
        return ErrorCode::DecoderOutOfRange;
    }
}

uint32_t WvDecoder::MaxBytesPerUnit() const
{
    return (10 * m_Channels * m_BytesPerSample);
}

uint64_t WvDecoder::UnitIndex() const
{
    return m_UnitIndex;
}

uint64_t WvDecoder::UnitCount() const
{
    return m_UnitCount;
}

EmAudioMode WvDecoder::AudioMode() const
{
    return AudioMode::Stereo;
}

int32_t WvDecoder::Channels() const
{
    return m_Channels;
}

int32_t WvDecoder::BitsPerSample() const
{
    return m_BitsPerSample;
}

int32_t WvDecoder::SampleRate() const
{
    return m_SampleRate;
}

int32_t WvDecoder::BitRate() const
{
    return m_BitRate; 
}

uint64_t WvDecoder::Duration() const
{
    return m_Duration;
}

static void format_samples (int bps, unsigned char *dst, int32_t *src, uint32_t samcnt)
{
    int32_t temp;

    switch (bps) {

    case 1:
        while (samcnt--)
            *dst++ = *src++ + 128;

        break;

    case 2:
        while (samcnt--) {
            *dst++ = (unsigned char) (temp = *src++);
            *dst++ = (unsigned char) (temp >> 8);
        }

        break;

    case 3:
        while (samcnt--) {
            *dst++ = (unsigned char) (temp = *src++);
            *dst++ = (unsigned char) (temp >> 8);
            *dst++ = (unsigned char) (temp >> 16);
        }

        break;

    case 4:
        while (samcnt--) {
            *dst++ = (unsigned char) (temp = *src++);
            *dst++ = (unsigned char) (temp >> 8);
            *dst++ = (unsigned char) (temp >> 16);
            *dst++ = (unsigned char) (temp >> 24);
        }

        break;
    }
}
