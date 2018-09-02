#include <string.h>
#include <iostream>
#include <ffmpeg-strip-wma/avcodec.h>
#include <ffmpeg-strip-wma/avformat.h>

#include <plugin/DecoderProto.h>
using namespace mous;

namespace {
    struct Self {
        AVCodecContext* codec_context = nullptr;
        AVFormatContext* format_context = nullptr;

        uint64_t unit_index;
        uint64_t unit_count;

        int32_t channels;
        int32_t bits_per_sample;
        int32_t sample_rate;
        int32_t bit_rate;
        uint64_t duration;
    };
}

static void* Create() {
    avcodec_init();
    avcodec_register_all();
    av_register_all();
    return new Self();
}

static void Destroy(void* ptr) {
    Close(ptr);
    delete SELF;
}

ErrorCode Open(void* ptr, const char* url) {
    SELF->format_context = nullptr;
    int err = av_open_input_file(&SELF->format_context, url, nullptr, 0, nullptr);
    if (err < 0) {
        return ErrorCode::DecoderFailedToOpen;
    }

    for (int i = 0; i < SELF->format_context->nb_streams; ++i) {
        SELF->codec_context = &SELF->format_context->streams[i]->codec;
        if (SELF->codec_context->codec_type == CODEC_TYPE_AUDIO) {
            break;
        }
    }

    av_find_stream_info(SELF->format_context);
    AVCodec* codec = avcodec_find_decoder(SELF->codec_context->codec_id);
        if (!codec || avcodec_open(SELF->codec_context, codec)) {
        return ErrorCode::DecoderFailedToInit;
    }

    // read info
    SELF->channels = SELF->codec_context->channels;
    SELF->sample_rate = SELF->codec_context->sample_rate;
    SELF->bits_per_sample = SELF->codec_context->bits_per_sample;
    SELF->duration = SELF->format_context->duration;

    SELF->unit_index = 0;
    SELF->unit_count = SELF->duration;

    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (SELF->codec_context) {
        avcodec_close(SELF->codec_context);
        SELF->codec_context = nullptr;
    }
    if (SELF->format_context) {
        av_close_input_file(SELF->format_context);
        SELF->format_context = nullptr;
    }
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    *used = 0;
    *unit_count = 0;

    SELF->bit_rate = SELF->codec_context->bit_rate / 1000;

    if (SELF->unit_index >= SELF->unit_count) {
        SELF->unit_index = SELF->unit_count;
        *unit_count = SELF->unit_count;
        return ErrorCode::DecoderOutOfRange;
    }

    AVPacket packet;
    if (av_read_frame(SELF->format_context, &packet) < 0) {
        SELF->unit_index = SELF->unit_count;
        *unit_count = SELF->unit_count;
        return ErrorCode::DecoderOutOfRange;
    }

    *unit_count = packet.duration;
    SELF->unit_index += *unit_count;

    int insize = packet.size;
    uint8_t* inbuf = packet.data;
    while (insize > 0) {
        int outsize = 0;
        int len = avcodec_decode_audio(
            SELF->codec_context,
            reinterpret_cast<short*>(data),
            &outsize,
            inbuf,
            insize
        );
        if (len < 0) {
            break;
        }
        if (outsize <= 0) {
            continue;
        }
        insize -= len;
        inbuf += len;
        data += outsize;
        *used += outsize;
        if (packet.data) {
            av_free_packet(&packet);
        }
    }

    return ErrorCode::Ok;
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    if (index < SELF->unit_count) {
        SELF->unit_index = index;
        av_seek_frame(SELF->format_context, -1, index);
        return ErrorCode::Ok;
    } else {
        return ErrorCode::DecoderOutOfRange;
    }
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    return AVCODEC_MAX_AUDIO_FRAME_SIZE;
}

static uint64_t GetUnitIndex(void* ptr) {
    return SELF->unit_index;
}

static uint64_t GetUnitCount(void* ptr) {
    return SELF->unit_count;
}

static AudioMode GetAudioMode(void* ptr) {
    return AudioMode::Stereo;
}

static int32_t GetChannels(void* ptr) {
    return SELF->channels;
}

static int32_t GetBitsPerSample(void* ptr) {
    return SELF->bits_per_sample;
}

static int32_t GetSampleRate(void* ptr) {
    return SELF->sample_rate;
}

static int32_t GetBitRate(void* ptr) {
    return SELF->bit_rate; 
}

static uint64_t GetDuration(void* ptr) {
    return SELF->duration / 1000;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

static const char** GetSuffixes(void* ptr) {
    (void) ptr;
    static const char* suffixes[] { "wma", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "wma", nullptr };
    return encodings;
}