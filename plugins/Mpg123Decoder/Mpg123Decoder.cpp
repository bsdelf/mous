#include "Mpg123Decoder.h"
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
    if (m_pHandle != NULL) {
	mpg123_close(m_pHandle);
	mpg123_delete(m_pHandle);
    }
    mpg123_exit();
}

void Mpg123Decoder::GetFileSuffix(vector<string>& list) const
{
    list.clear();
    list.push_back("mp3");
}

ErrorCode Mpg123Decoder::Open(const std::string& url)
{
    if (m_pHandle == NULL)
	return MousDecoderFailedToInit;

    if (mpg123_open(m_pHandle, url.c_str()) != MPG123_OK)
	return MousDecoderFailedToOpen;

    mpg123_scan(m_pHandle);
    mpg123_seek_frame(m_pHandle, 0, SEEK_END);
    m_UnitCount = mpg123_tellframe(m_pHandle);
    mpg123_seek_frame(m_pHandle, 0, SEEK_SET);

    if (m_UnitCount <= 0)
	return MousDecoderFailedToOpen;

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

    return MousOk;
}

void Mpg123Decoder::Close()
{
    if (m_pHandle != NULL)
	mpg123_close(m_pHandle);
}

bool Mpg123Decoder::IsFormatVaild() const
{
    return true;
}

ErrorCode Mpg123Decoder::ReadUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    if (m_UnitIndex < m_UnitCount) {
	unsigned char* _data;
	size_t _len;
	mpg123_decode_frame(m_pHandle, (off_t*)&m_UnitIndex, &_data, &_len);
	memcpy(data, _data, _len);
	used = _len;
	unitCount = 1;
	++m_UnitIndex;
	return MousOk;
    } else {
	used = 0;
	return MousDecoderOutOfRange;
    }
}

ErrorCode Mpg123Decoder::SetUnitIndex(uint64_t index)
{
    if (index < m_UnitCount) {
	m_UnitIndex = index;
	mpg123_seek_frame(m_pHandle, m_UnitIndex, SEEK_SET);
	return MousOk;
    } else {
	return MousDecoderOutOfRange;
    }
}

uint32_t Mpg123Decoder::GetMaxBytesPerUnit() const
{
    return m_MaxBytesPerUnit;
}

uint64_t Mpg123Decoder::GetUnitIndex() const
{
    return m_UnitIndex;
}

uint64_t Mpg123Decoder::GetUnitCount() const
{
    return m_UnitCount;
}

AudioMode Mpg123Decoder::GetAudioMode() const
{
    return MousStereo;
}

int32_t Mpg123Decoder::GetChannels() const
{
    return m_Channels;
}

int32_t Mpg123Decoder::GetBitsPerSample() const
{
    return m_BitsPerSample;
}

int32_t Mpg123Decoder::GetSampleRate() const
{
    return m_SampleRate;
}

uint64_t Mpg123Decoder::GetDuration() const
{
    return m_Duration;
}
