#include "Mp3Decoder.h"

Mp3Decoder::Mp3Decoder()
{
    int ret = mpg123_init();
    mpg123_pars* pars = mpg123_new_pars(&ret);
    const char** decoders = mpg123_supported_decoders();

    m_pHandle = mpg123_parnew(pars, decoders[0], &ret);
    mpg123_delete_pars(pars);

    mpg123_param(m_pHandle, MPG123_FLAGS, MPG123_QUIET, 0);

    m_msPerUnit = mpg123_outblock(m_pHandle);
}

Mp3Decoder::~Mp3Decoder()
{
    if (m_pHandle != NULL) {
	mpg123_close(m_pHandle);
	mpg123_delete(m_pHandle);
    }
    mpg123_exit();
}

const std::vector<std::string> Mp3Decoder::GetFileSuffix() const
{
    vector<string> suffix;
    suffix.push_back("mp3");
    suffix.push_back("MP3");
    suffix.push_back("Mp3");
    return suffix;
}

ErrorCode Mp3Decoder::Open(const std::string& url)
{
    if (m_pHandle == NULL)
	return MousDecoderFailedToInit;

    if (mpg123_open(m_pHandle, url.c_str()) != MPG123_OK)
	return MousDecoderFailedToOpen;

    mpg123_scan(m_pHandle);
    mpg123_seek_frame(m_pHandle, 0, SEEK_END);
    m_unitCount = mpg123_tellframe(m_pHandle);
    mpg123_seek_frame(m_pHandle, 0, SEEK_SET);

    if (m_unitCount <= 0)
	return MousDecoderFailedToOpen;

    long rate;
    int channels;
    int encoding;
    mpg123_getformat(m_pHandle, &rate, &channels, &encoding);

    m_duration = mpg123_tpf(m_pHandle) * 1000.f * m_unitCount;
    m_unitIndex = 0;

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

ErrorCode Mp3Decoder::ReadUnit()
{
    ++m_unitIndex;
    return MousOk;
}

ErrorCode Mp3Decoder::SetUnitIndex(uint32_t index)
{
    m_unitIndex = index;
    return MousOk;
}

uint32_t Mp3Decoder::GetUnitIndex() const
{
    return 0;
}

uint32_t Mp3Decoder::GetUnitCount() const
{
    return 0;
}

uint32_t Mp3Decoder::GetMsPerUnit() const
{
    return 0;
}

AudioMode Mp3Decoder::GetAudioMode() const
{
    return MousStereo;
}

uint32_t Mp3Decoder::GetBitRate() const
{
    return 0;
}

uint32_t Mp3Decoder::GetSampleRate() const
{
    return 0;
}

uint64_t Mp3Decoder::GetDuration() const
{
    return 0;
}
