#include "MacDecoder.h"

#include <stdio.h>

MacDecoder::~MacDecoder()
{
    Close();
}

vector<string> MacDecoder::FileSuffix() const
{
    return { "ape" };
}

ErrorCode MacDecoder::Open(const string& url)
{
    int err = ERROR_SUCCESS;

    APE::CSmartPtr<wchar_t> spFileName = APE::CAPECharacterHelper::GetUTF16FromANSI(url.c_str());
    m_pDecompress = CreateIAPEDecompress(spFileName, &err);

    if (m_pDecompress == nullptr || err != ERROR_SUCCESS)
        return ErrorCode::DecoderFailedToOpen;

    m_Channels = m_pDecompress->GetInfo(APE::APE_INFO_CHANNELS);
    m_SampleRate = m_pDecompress->GetInfo(APE::APE_INFO_SAMPLE_RATE);
    m_BitsPerSample = m_pDecompress->GetInfo(APE::APE_INFO_BITS_PER_SAMPLE);

    m_Duration = m_pDecompress->GetInfo(APE::APE_INFO_LENGTH_MS);

    m_BlockAlign = m_pDecompress->GetInfo(APE::APE_INFO_BLOCK_ALIGN);
    m_BlocksPerFrame = m_pDecompress->GetInfo(APE::APE_INFO_BLOCKS_PER_FRAME);
    m_BlockCount = m_pDecompress->GetInfo(APE::APE_INFO_TOTAL_BLOCKS);
    m_BlocksPerRead = m_BlocksPerFrame / 16;

    m_BlockIndex = 0;

    return ErrorCode::Ok;
}

void MacDecoder::Close()
{
    if (m_pDecompress != nullptr) {
        delete m_pDecompress; 
        m_pDecompress = nullptr;
    }
}

ErrorCode MacDecoder::DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    if (m_BlockIndex < m_BlockCount) {
        m_BitRate = m_pDecompress->GetInfo(APE::APE_DECOMPRESS_CURRENT_BITRATE);

        APE::intn blocksRecv = 0;
        auto ret = m_pDecompress->GetData(data, m_BlocksPerRead, &blocksRecv);
        switch (ret) {
            case ERROR_SUCCESS:
            {
                used = blocksRecv * m_BlockAlign;
                unitCount = blocksRecv;
                m_BlockIndex += blocksRecv;
                return ErrorCode::Ok;
            }
                break;

            case ERROR_INVALID_CHECKSUM:
                printf("FATAL: mac invalid checksum!\n");
                break;

            default:
                printf("FATAL: mac bad unit!\n");
                break;
        }
    } 

    printf("FATAL: mac hit end or error occured!\n");

    used = 0;
    unitCount = m_BlockCount;
    m_BlockIndex = m_BlockCount;

    return ErrorCode::DecoderOutOfRange;
}

ErrorCode MacDecoder::SetUnitIndex(uint64_t index)
{
    m_pDecompress->Seek(index);
    m_BlockIndex = index;
    return ErrorCode::Ok;
}

uint32_t MacDecoder::MaxBytesPerUnit() const
{
    return m_BlocksPerRead * m_BlockAlign;
}

uint64_t MacDecoder::UnitIndex() const
{
    return m_BlockIndex;
}

uint64_t MacDecoder::UnitCount() const
{
    return m_BlockCount;
}

AudioMode MacDecoder::AudioMode() const
{
    return AudioMode::Stereo;
}

int32_t MacDecoder::Channels() const
{
    return m_Channels;
}

int32_t MacDecoder::BitsPerSample() const
{
    return m_BitsPerSample;
}

int32_t MacDecoder::SampleRate() const
{
    return m_SampleRate;
}

int32_t MacDecoder::BitRate() const
{
    return m_BitRate;
}

uint64_t MacDecoder::Duration() const
{
    return m_Duration;
}
