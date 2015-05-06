#include "OggDecoder.h"
#include <string.h>

OggDecoder::OggDecoder()
{
}

OggDecoder::~OggDecoder()
{
}

vector<string> OggDecoder::FileSuffix() const
{
    return { "ogg" };
}

EmErrorCode OggDecoder::Open(const std::string& url)
{
    if (ov_fopen(url.c_str(), &m_File) != 0)
        return ErrorCode::DecoderFailedToOpen;
    if (ov_streams(&m_File) < 1)
        return ErrorCode::DecoderFailedToInit;

    m_BitStream = 0;

    m_UnitCount = ov_pcm_total(&m_File, 0);
    m_UnitIndex = 0;

    m_Duration = ov_time_total(&m_File, -1) * 1000;
    m_Channels = ov_info(&m_File, 0)->channels;
    m_SampleRate = ov_info(&m_File, 0)->rate;
    m_BitsPerSample = 16;

    return ErrorCode::Ok;
}

void OggDecoder::Close()
{
    ov_clear(&m_File);
}

bool OggDecoder::IsFormatVaild() const
{
    return true;
}

EmErrorCode OggDecoder::DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    if (m_UnitIndex < m_UnitCount) {
        long bytes = ov_read(&m_File, data, MaxBytesPerUnit(), 0, 2, 1, &m_BitStream);
        switch (bytes) {
            case OV_HOLE:
            case OV_EBADLINK:
            case OV_EINVAL:
            case 0:
                break;

            default:
            {
                m_BitRate = ov_info(&m_File, m_BitStream)->bitrate_nominal/1000;

                used = bytes;
                unitCount = ov_pcm_tell(&m_File) - m_UnitIndex;
                m_UnitIndex += unitCount;
                return ErrorCode::Ok;
            }
                break;
        }
    }

    m_BitRate = 0;
    used = 0;
    unitCount = m_UnitCount;
    return ErrorCode::DecoderOutOfRange;
}

EmErrorCode OggDecoder::SetUnitIndex(uint64_t index)
{
    if (index < m_UnitCount && ov_pcm_seek(&m_File, index) == 0) {
        m_UnitIndex = index;
        return ErrorCode::Ok;
    } else {
        return ErrorCode::DecoderOutOfRange;
    }
}

uint32_t OggDecoder::MaxBytesPerUnit() const
{
    return 4096 * 2;
}

uint64_t OggDecoder::UnitIndex() const
{
    return m_UnitIndex;
}

uint64_t OggDecoder::UnitCount() const
{
    return m_UnitCount;
}

EmAudioMode OggDecoder::AudioMode() const
{
    return AudioMode::Stereo;
}

int32_t OggDecoder::Channels() const
{
    return m_Channels;
}

int32_t OggDecoder::BitsPerSample() const
{
    return m_BitsPerSample;
}

int32_t OggDecoder::SampleRate() const
{
    return m_SampleRate;
}

int32_t OggDecoder::BitRate() const
{
    return m_BitRate; 
}

uint64_t OggDecoder::Duration() const
{
    return m_Duration;
}
