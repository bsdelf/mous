#include "Mp3Decoder.h"
#include <string.h>

Mp3Decoder::Mp3Decoder()
{
    int ret = mpg123_init();
    mpg123_pars* pars = mpg123_new_pars(&ret);
    const char** decoders = mpg123_supported_decoders();

    m_pHandle = mpg123_parnew(pars, decoders[0], &ret);
    mpg123_delete_pars(pars);

    mpg123_param(m_pHandle, MPG123_FLAGS, MPG123_QUIET, 0);

    m_MaxBytesPerUnit = mpg123_outblock(m_pHandle);
}

Mp3Decoder::~Mp3Decoder()
{
    if (m_pHandle != NULL) {
	mpg123_close(m_pHandle);
	mpg123_delete(m_pHandle);
    }
    mpg123_exit();
}

void Mp3Decoder::GetFileSuffix(vector<string>& list) const
{
    list.clear();
    list.push_back("mp3");
}

ErrorCode Mp3Decoder::Open(const std::string& url)
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

void Mp3Decoder::Close()
{
    if (m_pHandle != NULL)
	mpg123_close(m_pHandle);
}

bool Mp3Decoder::IsFormatVaild() const
{
    return true;
}

ErrorCode Mp3Decoder::ReadUnit(char* data, uint32_t& used, uint32_t& unitCount)
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

ErrorCode Mp3Decoder::SetUnitIndex(uint64_t index)
{
    if (index < m_UnitCount) {
	m_UnitIndex = index;
	mpg123_seek_frame(m_pHandle, m_UnitIndex, SEEK_SET);
	return MousOk;
    } else {
	return MousDecoderOutOfRange;
    }
}

uint32_t Mp3Decoder::GetMaxBytesPerUnit() const
{
    return m_MaxBytesPerUnit;
}

uint64_t Mp3Decoder::GetUnitIndex() const
{
    return m_UnitIndex;
}

uint64_t Mp3Decoder::GetUnitCount() const
{
    return m_UnitCount;
}

AudioMode Mp3Decoder::GetAudioMode() const
{
    return MousStereo;
}

int32_t Mp3Decoder::GetChannels() const
{
    return m_Channels;
}

int32_t Mp3Decoder::GetBitsPerSample() const
{
    return m_BitsPerSample;
}

int32_t Mp3Decoder::GetSampleRate() const
{
    return m_SampleRate;
}

uint64_t Mp3Decoder::GetDuration() const
{
    return m_Duration;
}
