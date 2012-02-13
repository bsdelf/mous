#include "AacDecoder.h"

AacDecoder::AacDecoder()
{

}

AacDecoder::~AacDecoder()
{
}

void AacDecoder::GetFileSuffix(vector<string>& list) const
{
    list.clear();
    list.push_back("m4a");
    list.push_back("aac");
}

ErrorCode AacDecoder::Open(const string& url)
{
    return MousOk;
}

void AacDecoder::Close()
{
}

bool AacDecoder::IsFormatVaild() const
{
    return true;
}

ErrorCode AacDecoder::ReadUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    return MousOk;
}

ErrorCode AacDecoder::SetUnitIndex(uint64_t index)
{
    return MousOk;
}

uint32_t AacDecoder::GetMaxBytesPerUnit() const
{
    return m_BlocksPerRead * m_BlockAlign;
}

uint64_t AacDecoder::GetUnitIndex() const
{
    return m_BlockIndex;
}

uint64_t AacDecoder::GetUnitCount() const
{
    return m_BlockCount;
}

AudioMode AacDecoder::GetAudioMode() const
{
    return MousStereo;
}

uint32_t AacDecoder::GetChannels() const
{
    return m_Channels;
}

uint32_t AacDecoder::GetBitsPerSample() const
{
    return m_BitsPerSample;
}

uint32_t AacDecoder::GetSampleRate() const
{
    return m_SampleRate;
}

uint64_t AacDecoder::GetDuration() const
{
    return m_Duration;
}
