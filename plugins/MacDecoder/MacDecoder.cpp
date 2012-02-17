#include "MacDecoder.h"
#include <mac/CharacterHelper.h>

MacDecoder::MacDecoder()
{

}

MacDecoder::~MacDecoder()
{
    if (m_pDecompress != NULL) {
	delete m_pDecompress;
	m_pDecompress = NULL;
    }
}

void MacDecoder::GetFileSuffix(vector<string>& list) const
{
    list.clear();
    list.push_back("ape");
}

EmErrorCode MacDecoder::Open(const string& url)
{
    int err;
    str_utf16* pFileName = GetUTF16FromANSI(url.c_str());
    m_pDecompress = CreateIAPEDecompress(pFileName, &err);
    delete[] pFileName;

    if (m_pDecompress == NULL || err != ERROR_SUCCESS)
	return ErrorCode::DecoderFailedToOpen;

    m_Channels = m_pDecompress->GetInfo(APE_INFO_CHANNELS);
    m_SampleRate = m_pDecompress->GetInfo(APE_INFO_SAMPLE_RATE);
    m_BitsPerSample = m_pDecompress->GetInfo(APE_INFO_BITS_PER_SAMPLE);

    m_Duration = m_pDecompress->GetInfo(APE_INFO_LENGTH_MS);

    m_BlockAlign = m_pDecompress->GetInfo(APE_INFO_BLOCK_ALIGN);
    m_BlocksPerFrame = m_pDecompress->GetInfo(APE_INFO_BLOCKS_PER_FRAME);
    m_BlockCount = m_pDecompress->GetInfo(APE_INFO_TOTAL_BLOCKS);
    m_BlocksPerRead = m_BlocksPerFrame / 16;

    m_BlockIndex = 0;

    return ErrorCode::Ok;
}

void MacDecoder::Close()
{
    delete m_pDecompress; 
    m_pDecompress = NULL;
}

bool MacDecoder::IsFormatVaild() const
{
    return true;
}

EmErrorCode MacDecoder::ReadUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    int blocksRecv = 0;

    m_BitRate = m_pDecompress->GetInfo(APE_DECOMPRESS_CURRENT_BITRATE);

    if (m_pDecompress->GetData(data, m_BlocksPerRead, &blocksRecv) == ERROR_SUCCESS) {
	m_BlockIndex += blocksRecv;
	used = blocksRecv * m_BlockAlign;
	unitCount = blocksRecv;
    } else {
	used = 0;
    }
    return ErrorCode::Ok;
}

EmErrorCode MacDecoder::SetUnitIndex(uint64_t index)
{
    m_pDecompress->Seek(index);
    m_BlockIndex = index;
    return ErrorCode::Ok;
}

uint32_t MacDecoder::GetMaxBytesPerUnit() const
{
    return m_BlocksPerRead * m_BlockAlign;
}

uint64_t MacDecoder::GetUnitIndex() const
{
    return m_BlockIndex;
}

uint64_t MacDecoder::GetUnitCount() const
{
    return m_BlockCount;
}

EmAudioMode MacDecoder::GetAudioMode() const
{
    return AudioMode::Stereo;
}

int32_t MacDecoder::GetChannels() const
{
    return m_Channels;
}

int32_t MacDecoder::GetBitsPerSample() const
{
    return m_BitsPerSample;
}

int32_t MacDecoder::GetSampleRate() const
{
    return m_SampleRate;
}

int32_t MacDecoder::GetBitRate() const
{
    return m_BitRate;
}

uint64_t MacDecoder::GetDuration() const
{
    return m_Duration;
}
