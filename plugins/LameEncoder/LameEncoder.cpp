#include "LameEncoder.h"
#include <scx/Conv.hpp>

LameEncoder::LameEncoder():
    m_gfp(NULL),
    m_OutputFile(NULL),
    m_BitsPerSample(0),
    m_EncodeBuffer(NULL),
    m_EncodeBufferSize(0),
    m_MediaTag(NULL)
{
    m_Quality.desc = "Quality\n0=best(very slow), 9 worst";
    m_Quality.min = 0;
    m_Quality.max = 9;
    m_Quality.defaultVal = 5;
    m_Quality.userVal = 5;

    m_BitRate.desc = "Bit Rate";
    int rates[] = { 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 };
    m_BitRate.enumedVal.assign(rates, rates + sizeof(rates)/sizeof(int));
    m_BitRate.defaultChoice = sizeof(rates)/sizeof(int) - 4;
    m_BitRate.userChoice = sizeof(rates)/sizeof(int) - 4;

    m_ReplayGain.desc = "ReplayGain";
    m_ReplayGain.detail = "Perform ReplayGain Analysis";
    m_ReplayGain.defaultChoice = true;
    m_ReplayGain.userChoice = true;
}

LameEncoder::~LameEncoder()
{
    CloseOutput();
}

const char* LameEncoder::FileSuffix() const
{
    return "mp3";
}

EmErrorCode LameEncoder::OpenOutput(const std::string& path)
{
    m_OutputFile = ::fopen(path.c_str(), "wb+");

    if (m_OutputFile == NULL)
        return ErrorCode::EncoderFailedToOpen;

    // init lame
    m_gfp = ::lame_init();

    ::lame_set_quality(m_gfp, m_Quality.userVal);
    ::lame_set_brate(m_gfp, m_BitRate.enumedVal[m_BitRate.userChoice]);
    ::lame_set_mode(m_gfp, ::JOINT_STEREO);
    ::lame_set_findReplayGain(m_gfp, m_ReplayGain.userChoice ? 1 : 0);
    ::lame_set_asm_optimizations(m_gfp, MMX, 1);
    ::lame_set_asm_optimizations(m_gfp, SSE, 1);
    if (m_MediaTag != NULL) {
        lame_set_write_id3tag_automatic(m_gfp, 1);
        id3tag_init(m_gfp);
        id3tag_v2_only(m_gfp);
        id3tag_set_title(m_gfp, m_MediaTag->title.c_str());
        id3tag_set_artist(m_gfp, m_MediaTag->artist.c_str());
        id3tag_set_album(m_gfp, m_MediaTag->album.c_str());
        id3tag_set_comment(m_gfp, m_MediaTag->comment.c_str());
        id3tag_set_genre(m_gfp, m_MediaTag->genre.c_str());
        id3tag_set_year(m_gfp, scx::NumToStr(m_MediaTag->year).c_str());
        id3tag_set_track(m_gfp, scx::NumToStr(m_MediaTag->track).c_str());
    }
    int ret = ::lame_init_params(m_gfp);
    if (ret < 0)
        return ErrorCode::EncoderFailedToInit;

    return ErrorCode::Ok;
}

void LameEncoder::CloseOutput()
{
    if (m_OutputFile != NULL) {
        ::fclose(m_OutputFile);
        m_OutputFile = NULL;
    }

    if (m_gfp != NULL) {
        ::lame_close(m_gfp);
        m_gfp = NULL;
    }

    if (m_EncodeBuffer != NULL) {
        delete[] m_EncodeBuffer;
        m_EncodeBuffer = NULL;
        m_EncodeBufferSize = 0;
    }
}

EmErrorCode LameEncoder::Encode(char* buf, uint32_t len)
{
    // prepare buffer
    int samplesPerChannel = 
        len / ::lame_get_num_channels(m_gfp) / (m_BitsPerSample / 8); 
    int minBufferSize = 1.25 * samplesPerChannel + 7200;
    if (m_EncodeBufferSize < minBufferSize) {
        if (m_EncodeBuffer != NULL)
            delete[] m_EncodeBuffer;
        m_EncodeBuffer = new unsigned char[minBufferSize];
        m_EncodeBufferSize = minBufferSize;
    }

    // encode
    int ret = lame_encode_buffer_interleaved(
            m_gfp,
            (short int*)buf, samplesPerChannel,
            m_EncodeBuffer, m_EncodeBufferSize);

    if (ret >= 0) {
        if ((int)::fwrite(m_EncodeBuffer, 1, ret, m_OutputFile) == ret) {
            return ErrorCode::Ok;
        }
    }

    return ErrorCode::EncoderFailedToEncode;
}

EmErrorCode LameEncoder::FlushRest()
{
    int ret = lame_encode_flush(m_gfp, m_EncodeBuffer, m_EncodeBufferSize);
    if (ret >= 0) {
        if ((int)::fwrite(m_EncodeBuffer, 1, ret, m_OutputFile) == ret) {
            ::lame_mp3_tags_fid(m_gfp, m_OutputFile);
            return ErrorCode::Ok;
        }
    }
    return ErrorCode::EncoderFailedToFlush;
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

void LameEncoder::SetMediaTag(const MediaTag* tag)
{
    m_MediaTag = tag;
}

std::vector<const BaseOption*> LameEncoder::Options() const 
{
    std::vector<const BaseOption*> list(3);
    list.resize(3);
    list[0] = &m_Quality;
    list[1] = &m_BitRate;
    list[2] = &m_ReplayGain;
    return list;
}


