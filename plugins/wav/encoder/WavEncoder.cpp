#include "WavEncoder.h"
#include <cstring>

WavEncoder::WavEncoder()
{
    memset(&m_WavHeader, 0, sizeof(WavHeader));

    memcpy(m_WavHeader.riffId, "RIFF", 4);
    memcpy(m_WavHeader.riffType, "WAVE", 4);

    memcpy(m_WavHeader.formatId, "fmt ", 4);
    memcpy(m_WavHeader.dataId, "data", 4);

    m_WavHeader.formatTag = 0x0001;
}

WavEncoder::~WavEncoder()
{
}

const char* WavEncoder::FileSuffix() const
{
    return "wav";
}

ErrorCode WavEncoder::OpenOutput(const std::string& path)
{
    m_OutputFile.open(path.c_str(), ios::binary | ios::out );
    if (!m_OutputFile.is_open())
        return ErrorCode::EncoderFailedToOpen;

    m_OutputFile.write((char*)&m_WavHeader, sizeof(WavHeader));
    return ErrorCode::Ok;
}

void WavEncoder::CloseOutput()
{
    m_WavHeader.dataChunkLen = (uint32_t)m_OutputFile.tellg() - (uint32_t)sizeof(WavHeader);
    m_WavHeader.lenAfterRiff = m_WavHeader.dataChunkLen + 36;
    m_WavHeader.formatChunkLen = 16;//m_WavHeader.dataChunkLen + 24;
    m_WavHeader.blockAlign = m_WavHeader.channels * ((m_WavHeader.bitsPerSample + 7) / 8);
    m_WavHeader.avgBytesPerSec = m_WavHeader.blockAlign * m_WavHeader.sampleRate;
    m_OutputFile.seekg(0, ios::beg);
    m_OutputFile.write((char*)&m_WavHeader, sizeof(WavHeader));

    if (m_OutputFile.is_open())
        m_OutputFile.close();
}

ErrorCode WavEncoder::Encode(char* buf, uint32_t len)
{
    m_OutputFile.write(buf, len);
    return ErrorCode::Ok;
}

ErrorCode WavEncoder::Flush()
{
    return ErrorCode::Ok;
}

void WavEncoder::SetChannels(int32_t channels)
{
    m_WavHeader.channels = channels;
}

void WavEncoder::SetSampleRate(int32_t sampleRate)
{
    m_WavHeader.sampleRate = sampleRate;
}

void WavEncoder::SetBitsPerSample(int32_t bitsPerSample)
{
    m_WavHeader.bitsPerSample = bitsPerSample;
}
