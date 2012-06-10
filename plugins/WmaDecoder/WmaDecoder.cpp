#include "WmaDecoder.h"
#include <string.h>
#include <iostream>

WmaDecoder::WmaDecoder():
    m_CodecCtx(NULL),
    m_FormatCtx(NULL)
{
    avcodec_init();
    avcodec_register_all();
    av_register_all();
}

WmaDecoder::~WmaDecoder()
{
}

vector<string> WmaDecoder::FileSuffix() const
{
    vector<string> list;
    list.push_back("wma");
    return list;
}

EmErrorCode WmaDecoder::Open(const std::string& url)
{
    m_FormatCtx = NULL;
    int err = av_open_input_file(&m_FormatCtx, url.c_str(), NULL, 0, NULL);
    if (err < 0)
        return ErrorCode::DecoderFailedToOpen;

    for (int i = 0; i < m_FormatCtx->nb_streams; ++i) {
        m_CodecCtx = &m_FormatCtx->streams[i]->codec;
        if (m_CodecCtx->codec_type == CODEC_TYPE_AUDIO)
            break;
    }

    av_find_stream_info(m_FormatCtx);
    AVCodec* codec = avcodec_find_decoder(m_CodecCtx->codec_id);
    
    // open it
    if (codec == NULL || avcodec_open(m_CodecCtx, codec))
        return ErrorCode::DecoderFailedToInit;

    // read info
    m_Channels = m_CodecCtx->channels;
    m_SampleRate = m_CodecCtx->sample_rate;
    m_BitsPerSample = m_CodecCtx->bits_per_sample;
    m_Duration = m_FormatCtx->duration;

    m_UnitIndex = 0;
    m_UnitCount = m_Duration;

    return ErrorCode::Ok;
}

void WmaDecoder::Close()
{
    if (m_CodecCtx != NULL) {
        avcodec_close(m_CodecCtx);
        m_CodecCtx = NULL;
    }
    if (m_FormatCtx != NULL) {
        av_close_input_file(m_FormatCtx);
        m_FormatCtx = NULL;
    }
}

bool WmaDecoder::IsFormatVaild() const
{
    return true;
}

EmErrorCode WmaDecoder::DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    used = 0;
    unitCount = 0;

    m_BitRate = m_CodecCtx->bit_rate / 1000;

    if (m_UnitIndex < m_UnitCount) {
        AVPacket packet;
        if (av_read_frame(m_FormatCtx, &packet) < 0) {
            return ErrorCode::DecoderOutOfRange;
        }

        unitCount = packet.duration;
        m_UnitIndex += unitCount;

        int insize = packet.size;
        uint8_t* inbuf = packet.data;
        while (insize > 0) {
            int outsize = 0;
            int len = avcodec_decode_audio(m_CodecCtx, (short*)data, &outsize, inbuf, insize);
            if (len < 0)
                break;
            if (outsize <= 0)
                continue;
            insize -= len;
            inbuf += len;
            data += outsize;
            used += outsize;
            if (packet.data != NULL)
                av_free_packet(&packet);
        }
        return ErrorCode::Ok;
    } else {
        used = 0;
        return ErrorCode::DecoderOutOfRange;
    }
}

EmErrorCode WmaDecoder::SetUnitIndex(uint64_t index)
{
    if (index < m_UnitCount) {
        m_UnitIndex = index;
        av_seek_frame(m_FormatCtx, -1, index);
        return ErrorCode::Ok;
    } else {
        return ErrorCode::DecoderOutOfRange;
    }
}

uint32_t WmaDecoder::MaxBytesPerUnit() const
{
    return AVCODEC_MAX_AUDIO_FRAME_SIZE;
}

uint64_t WmaDecoder::UnitIndex() const
{
    return m_UnitIndex;
}

uint64_t WmaDecoder::UnitCount() const
{
    return m_UnitCount;
}

EmAudioMode WmaDecoder::AudioMode() const
{
    return AudioMode::Stereo;
}

int32_t WmaDecoder::Channels() const
{
    return m_Channels;
}

int32_t WmaDecoder::BitsPerSample() const
{
    return m_BitsPerSample;
}

int32_t WmaDecoder::SampleRate() const
{
    return m_SampleRate;
}

int32_t WmaDecoder::BitRate() const
{
    return m_BitRate; 
}

uint64_t WmaDecoder::Duration() const
{
    return m_Duration / 1000;
}
