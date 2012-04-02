#include "LameEncoder.h"
#include <stdio.h>

LameEncoder::LameEncoder():
    m_gfp(NULL),
    m_BitsPerSample(0),
    m_EncodeBuffer(NULL),
    m_EncodeBufferSize(0)
{
    //==== options
    m_Quality.type = OptionType::RangedInt;
    m_Quality.desc = "Quality:\n0=best(very slow), 9 worst.";
    m_Quality.min = 0;
    m_Quality.max = 9;
    m_Quality.defaultVal = 5;
    m_Quality.userVal = 5;

    m_BitRate.type = OptionType::EnumedInt;
    m_BitRate.desc = "Bit Rate:";
    int rates[] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
    m_BitRate.enumedVal.assign(rates, rates + sizeof(rates)/sizeof(int));
    m_BitRate.defaultChoice = sizeof(rates)/sizeof(int) - 4;
    m_BitRate.userChoice = sizeof(rates)/sizeof(int) - 4;

    //==== init lame
}

LameEncoder::~LameEncoder()
{
    if (m_EncodeBuffer != NULL)
        delete[] m_EncodeBuffer;
}

EmErrorCode LameEncoder::OpenOutput(const std::string& path)
{
    m_FileName = path;

    m_OutputFile.open(path.c_str(), ios::binary | ios::out );
    if (!m_OutputFile.is_open())
        return ErrorCode::EncoderFailedToOpen;

    m_gfp = ::lame_init();
    ::lame_set_quality(m_gfp, m_Quality.userVal);
    ::lame_set_brate(m_gfp, m_BitRate.enumedVal[m_BitRate.userChoice]);
    ::lame_set_mode(m_gfp, ::JOINT_STEREO);
    int ret = ::lame_init_params(m_gfp);
    if (ret < 0)
        return ErrorCode::EncoderFailedToInit;

    return ErrorCode::Ok;
}

void LameEncoder::CloseOutput()
{
    if (m_OutputFile.is_open()) {
        m_OutputFile.close();
        FILE* file = ::fopen(m_FileName.c_str(), "aw");
        lame_mp3_tags_fid(m_gfp, file);
        ::fclose(file);
    }

    if (m_gfp != NULL) {
        ::lame_close(m_gfp);
        m_gfp = NULL;
    }
}


EmErrorCode LameEncoder::Encode(char* buf, uint32_t len)
{
    //==== prepare buffer
    int samplesPerChannel = 
        len / ::lame_get_num_channels(m_gfp) / (m_BitsPerSample / 8); 
    int minBufferSize = 1.25 * samplesPerChannel + 7200;
    if (m_EncodeBufferSize < minBufferSize) {
        if (m_EncodeBuffer != NULL)
            delete[] m_EncodeBuffer;
        m_EncodeBuffer = new unsigned char[minBufferSize];
        m_EncodeBufferSize = minBufferSize;
    }

    //==== encode
    int ret = lame_encode_buffer_interleaved(
            m_gfp,
            (short int*)buf, samplesPerChannel,
            m_EncodeBuffer, m_EncodeBufferSize);
    m_OutputFile.write((char*)m_EncodeBuffer, ret);
    return ret >= 0 ? ErrorCode::Ok : ErrorCode::EncoderFailedToEncode;
}

EmErrorCode LameEncoder::FlushRest()
{
    int ret = lame_encode_flush(m_gfp, m_EncodeBuffer, m_EncodeBufferSize);
    m_OutputFile.write((char*)m_EncodeBuffer, ret);
    return ret >= 0 ? ErrorCode::Ok : ErrorCode::EncoderFailedToFlush;
}

void LameEncoder::SetChannels(int32_t channels)
{
    ::lame_set_num_channels(m_gfp, channels);
}

void LameEncoder::SetSampleRate(int32_t sampleRate)
{
    ::lame_set_num_samples(m_gfp, sampleRate);
}

void LameEncoder::SetBitsPerSample(int32_t bitsPerSample)
{
    m_BitsPerSample = bitsPerSample;
}

bool LameEncoder::GetOptions(std::vector<const BaseOption*>& list) const 
{
    list.clear();
    list.resize(2);
    list[0] = &m_Quality;
    list[1] = &m_BitRate;
    return true;
}


