#include "OggEncoder.h"

OggEncoder::OggEncoder():
    m_OutputFile(nullptr),
    m_Channels(0),
    m_SampleRate(0),
    m_BitsPerSample(0),
    m_MediaTag(nullptr)
{
    m_Quality.desc = "Quality Level";
    m_Quality.min = -1;
    m_Quality.max = 10;
    m_Quality.defaultVal = 5;
    m_Quality.userVal = 5;
    m_Quality.point = 10;

    m_BitRate.desc = "Bit Rate";
    int rates[] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
    m_BitRate.enumedVal.assign(rates, rates + sizeof(rates)/sizeof(int));
    m_BitRate.defaultChoice = sizeof(rates)/sizeof(int) - 4;
    m_BitRate.userChoice = sizeof(rates)/sizeof(int) - 4;

    m_VbrOrAbr.desc = "Encode with";
    m_VbrOrAbr.groups.resize(2);
    m_VbrOrAbr.groups[0].first = "VBR (Recommended)";
    m_VbrOrAbr.groups[0].second.push_back(&m_Quality);
    m_VbrOrAbr.groups[1].first = "ABR (Be careful)";
    m_VbrOrAbr.groups[1].second.push_back(&m_BitRate);
    m_VbrOrAbr.defaultUse = 0;
    m_VbrOrAbr.userUse = 0;
}

OggEncoder::~OggEncoder()
{
    CloseOutput();
}

const char* OggEncoder::FileSuffix() const
{
    return "ogg";
}

ErrorCode OggEncoder::OpenOutput(const std::string& path)
{
    m_OutputFile = ::fopen(path.c_str(), "wb+");
    if (m_OutputFile == nullptr)
        return ErrorCode::EncoderFailedToOpen;

    int ret = 0;
    vorbis_info_init(&m_VobisInfo);
    if (m_VbrOrAbr.userUse == 0) {
        double point = m_Quality.point;
        ret = vorbis_encode_init_vbr(&m_VobisInfo, m_Channels, m_SampleRate, m_Quality.userVal/point);
    }


    if (ret != 0)
        return ErrorCode::EncoderFailedToInit;

    return ErrorCode::Ok;
}

void OggEncoder::CloseOutput()
{
    if (m_OutputFile != nullptr) {
        ::fclose(m_OutputFile);
        m_OutputFile = nullptr;
    }
}

ErrorCode OggEncoder::Encode(char* buf, uint32_t len)
{
    /*
    if (ret >= 0) {
        if ((int)::fwrite(m_EncodeBuffer, 1, ret, m_OutputFile) == ret) {
            return ErrorCode::Ok;
        }
    }
    */

    return ErrorCode::EncoderFailedToEncode;
}

ErrorCode OggEncoder::Flush()
{
    /*
    int ret = lame_encode_flush(m_gfp, m_EncodeBuffer, m_EncodeBufferSize);
    if (ret >= 0) {
        if ((int)::fwrite(m_EncodeBuffer, 1, ret, m_OutputFile) == ret) {
            ::lame_mp3_tags_fid(m_gfp, m_OutputFile);
            return ErrorCode::Ok;
        }
    }
    */
    return ErrorCode::EncoderFailedToFlush;
}

void OggEncoder::SetChannels(int32_t channels)
{
    m_Channels = channels;
}

void OggEncoder::SetSampleRate(int32_t sampleRate)
{
    m_SampleRate = sampleRate;
}

void OggEncoder::SetBitsPerSample(int32_t bitsPerSample)
{
    m_BitsPerSample = bitsPerSample;
}

void OggEncoder::SetMediaTag(const MediaTag* tag)
{
    m_MediaTag = tag;
}

std::vector<const BaseOption*> OggEncoder::Options() const 
{
    return {};
}
