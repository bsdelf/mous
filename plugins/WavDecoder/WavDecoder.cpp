#include "WavDecoder.h"
#include <cstring>

WavDecoder::WavDecoder():
    SAMPLES_PER_BLOCK(200),
    m_RawDataOffset(0),
    m_RawDataLength(0),
    m_SampleLength(0),
    m_BlockLength(0),
    m_BlockBuffer(NULL),
    m_BlockIndex(0),
    m_TotalBlocks(0)
{
    memset(&m_WavHeader, 0, sizeof(WavHeader));
}

WavDecoder::~WavDecoder()
{
    Close();
}

vector<string> WavDecoder::GetFileSuffix() const
{
    vector<string> list;
    list.push_back("wav");
    return list;
}

EmErrorCode WavDecoder::Open(const std::string& url)
{
    m_InputStream.open(url.c_str(), ios::binary);
    if (!m_InputStream.is_open())
        return ErrorCode::DecoderFailedToOpen;

    m_InputStream.seekg(0, ios::beg);
    m_InputStream.read((char*)&m_WavHeader, sizeof(WavHeader));
    
    if (memcmp(m_WavHeader.riffId, "RIFF", 4) != 0
            || memcmp(m_WavHeader.riffType, "WAVE", 4) != 0
            || memcmp(m_WavHeader.formatId, "fmt ", 4) != 0) {
        m_InputStream.close();
        return ErrorCode::DecoderFailedToInit;
    }

    m_RawDataOffset = sizeof(WavHeader);
    m_RawDataLength = m_WavHeader.dataChunkLen;

    m_SampleLength = m_WavHeader.channels * m_WavHeader.bitsPerSample / 8.f;
    m_BlockLength = m_SampleLength * SAMPLES_PER_BLOCK;
    m_BlockBuffer = new char[m_BlockLength];
    m_BlockIndex = 0;
    m_TotalBlocks = m_RawDataLength/m_BlockLength + (m_RawDataLength%m_BlockLength == 0 ? 0 : 1);

    m_Duration = (double)m_RawDataLength
        / (m_WavHeader.bitsPerSample / 8 * m_WavHeader.channels) 
        / m_WavHeader.sampleRate * 1000;

    return ErrorCode::Ok;
}

void WavDecoder::Close()
{
    m_InputStream.close();

    memset(&m_WavHeader, 0, sizeof(WavHeader));

    if (m_BlockBuffer != NULL) {
        delete[] m_BlockBuffer;
        m_BlockBuffer = NULL;
    }
}

bool WavDecoder::IsFormatVaild() const
{
    return true;
}

EmErrorCode WavDecoder::DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    m_InputStream.read(data, m_BlockLength);
    used = m_InputStream.gcount();
    unitCount = 1;
    return ErrorCode::Ok;
}

EmErrorCode WavDecoder::SetUnitIndex(uint64_t index)
{
    if ((size_t)index > m_TotalBlocks)
        return ErrorCode::DecoderOutOfRange;

    m_BlockIndex = index;
    m_InputStream.seekg(m_RawDataOffset + m_BlockIndex*m_BlockLength, ios::beg);
    return ErrorCode::Ok;
}

uint32_t WavDecoder::GetMaxBytesPerUnit() const
{
    return m_BlockLength;
}

uint64_t WavDecoder::GetUnitIndex() const
{
    return m_BlockIndex;
}

uint64_t WavDecoder::GetUnitCount() const
{
    return m_TotalBlocks;
}

EmAudioMode WavDecoder::GetAudioMode() const
{
    return m_WavHeader.channels == 1 ? AudioMode::Mono : AudioMode::Stereo;
}

int32_t WavDecoder::GetChannels() const
{
    return m_WavHeader.channels;
}

int32_t WavDecoder::GetBitsPerSample() const
{
    return m_WavHeader.bitsPerSample;
}

int32_t WavDecoder::GetSampleRate() const
{
    return m_WavHeader.sampleRate;
}

int32_t WavDecoder::GetBitRate() const
{
    return m_WavHeader.avgBytesPerSec/1024;
}

uint64_t WavDecoder::GetDuration() const
{
    return m_Duration;
}
