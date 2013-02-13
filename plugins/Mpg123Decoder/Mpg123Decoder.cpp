#include "Mpg123Decoder.h"

#include <unistd.h> // for off_t
#include <string.h>

Mpg123Decoder::Mpg123Decoder()
{
    int ret = mpg123_init();
    mpg123_pars* pars = mpg123_new_pars(&ret);
    const char** decoders = mpg123_supported_decoders();

    m_pHandle = mpg123_parnew(pars, decoders[0], &ret);
    mpg123_delete_pars(pars);

    mpg123_param(m_pHandle, MPG123_FLAGS, MPG123_QUIET, 0);

    m_MaxBytesPerUnit = mpg123_outblock(m_pHandle);
}

Mpg123Decoder::~Mpg123Decoder()
{
    if (m_pHandle != nullptr) {
        mpg123_close(m_pHandle);
        mpg123_delete(m_pHandle);
    }
    mpg123_exit();
}

vector<string> Mpg123Decoder::FileSuffix() const
{
    vector<string> list;
    list.push_back("mp3");
    return list;
}

EmErrorCode Mpg123Decoder::Open(const std::string& url)
{
    if (m_pHandle == nullptr)
        return ErrorCode::DecoderFailedToInit;

    if (mpg123_open(m_pHandle, url.c_str()) != MPG123_OK)
        return ErrorCode::DecoderFailedToOpen;

    mpg123_scan(m_pHandle);
    mpg123_seek_frame(m_pHandle, 0, SEEK_END);
    m_UnitCount = mpg123_tellframe(m_pHandle);
    mpg123_seek_frame(m_pHandle, 0, SEEK_SET);

    if (m_UnitCount <= 0)
        return ErrorCode::DecoderFailedToOpen;

    long sampleRate;
    int channels;
    int encoding;
    mpg123_getformat(m_pHandle, &sampleRate, &channels, &encoding);

    m_Channels = channels;
    m_SampleRate = sampleRate;
    m_BitsPerSample = (encoding == MPG123_ENC_SIGNED_16) || 
        (encoding = MPG123_ENC_UNSIGNED_16) || 
        (encoding == MPG123_ENC_16) ? 16: 8;

    m_Duration = mpg123_tpf(m_pHandle) * 1000.f * m_UnitCount;
    m_UnitIndex = 0;

    return ErrorCode::Ok;
}

void Mpg123Decoder::Close()
{
    if (m_pHandle != nullptr)
        mpg123_close(m_pHandle);
}

bool Mpg123Decoder::IsFormatVaild() const
{
    return true;
}

EmErrorCode Mpg123Decoder::DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    mpg123_frameinfo info;
    mpg123_info(m_pHandle, &info);
    m_BitRate = info.bitrate;

    if (m_UnitIndex < m_UnitCount) {
        unsigned char* _data;
        size_t _len;
        mpg123_decode_frame(m_pHandle, (off_t*)&m_UnitIndex, &_data, &_len);
        memcpy(data, _data, _len);
        used = _len;
        unitCount = 1;
        ++m_UnitIndex;
        return ErrorCode::Ok;
    } else {
        used = 0;
        return ErrorCode::DecoderOutOfRange;
    }
}

EmErrorCode Mpg123Decoder::SetUnitIndex(uint64_t index)
{
    if (index < m_UnitCount) {
        m_UnitIndex = index;
        mpg123_seek_frame(m_pHandle, m_UnitIndex, SEEK_SET);
        return ErrorCode::Ok;
    } else {
        return ErrorCode::DecoderOutOfRange;
    }
}

uint32_t Mpg123Decoder::MaxBytesPerUnit() const
{
    return m_MaxBytesPerUnit;
}

uint64_t Mpg123Decoder::UnitIndex() const
{
    return m_UnitIndex;
}

uint64_t Mpg123Decoder::UnitCount() const
{
    return m_UnitCount;
}

EmAudioMode Mpg123Decoder::AudioMode() const
{
    return AudioMode::Stereo;
}

int32_t Mpg123Decoder::Channels() const
{
    return m_Channels;
}

int32_t Mpg123Decoder::BitsPerSample() const
{
    return m_BitsPerSample;
}

int32_t Mpg123Decoder::SampleRate() const
{
    return m_SampleRate;
}

int32_t Mpg123Decoder::BitRate() const
{
    return m_BitRate; 
}

uint64_t Mpg123Decoder::Duration() const
{
    return m_Duration;
}
