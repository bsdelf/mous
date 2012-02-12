#include "ApeDecoder.h"
#include <mac/CharacterHelper.h>

ApeDecoder::ApeDecoder()
{

}

ApeDecoder::~ApeDecoder()
{
    if (m_pDecompress != NULL) {
	delete m_pDecompress;
	m_pDecompress = NULL;
    }
}

void ApeDecoder::GetFileSuffix(vector<string>& list) const
{
    list.clear();
    list.push_back("ape");
}

ErrorCode ApeDecoder::Open(const string& url)
{
    int err;
    str_utf16* pFileName = GetUTF16FromANSI(url.c_str());
    m_pDecompress = CreateIAPEDecompress(pFileName, &err);
    delete[] pFileName;

    if (m_pDecompress == NULL || err != ERROR_SUCCESS)
	return MousDecoderFailedToOpen;

    m_Channels = m_pDecompress->GetInfo(APE_INFO_CHANNELS);
    m_SampleRate = m_pDecompress->GetInfo(APE_INFO_SAMPLE_RATE);
    m_BitsPerSample = m_pDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE);

    m_Duration = m_pDecompress->GetInfo(APE_INFO_LENGTH_MS);

    m_BlockAlign = m_pDecompress->GetInfo(APE_INFO_BLOCK_ALIGN);
    m_BlocksPerFrame = m_pDecompress->GetInfo(APE_INFO_BLOCKS_PER_FRAME);
    m_BlockCount = m_pDecompress->GetInfo(APE_INFO_TOTAL_BLOCKS);

    m_BlockIndex = 0;

    return MousOk;
}

void ApeDecoder::Close()
{

}

bool ApeDecoder::IsFormatVaild() const
{
    return true;
}

ErrorCode ApeDecoder::ReadUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    int blocksRecv = 0;

    if (m_pDecompress->GetData(data, m_BlocksPerFrame/16, &blocksRecv) == ERROR_SUCCESS) {
	used = blocksRecv * m_BlockAlign;
	m_BlockIndex += blocksRecv;
	unitCount = blocksRecv;
    } else {
	used = 0;
    }
    return MousOk;
}

ErrorCode ApeDecoder::SetUnitIndex(uint64_t index)
{
    m_pDecompress->Seek(index);
    m_BlockIndex = index;
    return MousOk;
}

uint32_t ApeDecoder::GetMaxBytesPerUnit() const
{
    return m_BlocksPerFrame/16 * m_BlockAlign;
}

uint64_t ApeDecoder::GetUnitIndex() const
{
    return m_BlockIndex;
}

uint64_t ApeDecoder::GetUnitCount() const
{
    return m_BlockCount;
}

AudioMode ApeDecoder::GetAudioMode() const
{
    return MousStereo;
}

uint32_t ApeDecoder::GetChannels() const
{
    return m_Channels;
}

uint32_t ApeDecoder::GetBitsPerSample() const
{
    return m_BitsPerSample;
}

uint32_t ApeDecoder::GetSampleRate() const
{
    return m_SampleRate;
}

uint64_t ApeDecoder::GetDuration() const
{
    return m_Duration;
}
