#include <FLAC/stream_decoder.h>
#include <plugin/DecoderProto.h>
using namespace mous;

namespace {
    struct DecoderContext {
        char* buf = nullptr;
        int32_t buf_used = 0;
        int32_t samples_read = 0;
    };
}

struct Self {
    FLAC__StreamDecoder* decoder;
    DecoderContext decoder_context;

    uint64_t sample_index;
    uint64_t sample_count;

    int32_t channels;
    int32_t bits_per_sample;
    int32_t sample_rate;
    int32_t samples_per_frame;
    int32_t bit_rate;
    uint64_t duration;
};

static FLAC__StreamDecoderWriteStatus WriteCallback(
    const FLAC__StreamDecoder* decoder, 
    const FLAC__Frame* frame, 
    const FLAC__int32* const buffer[], 
    void* client_data) {
    (void) decoder;
    const unsigned int samples = frame->header.blocksize;
    const unsigned int channels = frame->header.channels;
    const unsigned int bits = frame->header.bits_per_sample;
    const auto bytes = bits >> 3;
    auto context = static_cast<DecoderContext*>(client_data);
    if (context->buf) {
        for (unsigned int ichannel = 0; ichannel < channels; ++ichannel) {
            auto channel = buffer[ichannel];
            auto data = context->buf + bytes * ichannel;
            for (unsigned int isample = 0; isample < samples; ++isample) {
                auto sample = channel[isample];
                switch (bits) {
                    case 8: {
                        data[0] = sample ^ 0x80;
                        break;
                    }
                    case 24: {
                        data[2] = sample >> 16;
                        //[[fallthrough]]
                        /* fall through */
                    }
                    case 16: {
                        data[1] = sample >> 8;
                        data[0] = sample;
                        break;
                    }
                    default: {
                        break;
                    }
                }
                data += bytes * channels;
            }
        }
        context->buf_used = channels * samples * bytes;
        context->samples_read = samples;
    }
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void ErrorCallback(
    const FLAC__StreamDecoder* decoder,
    FLAC__StreamDecoderErrorStatus status,
    void* client_data) {
    (void) decoder;
    (void) status;
    (void) client_data;
}

static void* Create() {
    auto self = new Self;
    self->decoder = FLAC__stream_decoder_new();
    return self;
}

static void Destroy(void* ptr) {
    Close(ptr);
    FLAC__stream_decoder_delete(SELF->decoder);
    delete SELF;
}

static ErrorCode Open(void* ptr, const char* url) {
    FLAC__stream_decoder_set_md5_checking(SELF->decoder, false);
    FLAC__stream_decoder_init_file(
        SELF->decoder,
        url,
        WriteCallback,
        nullptr,        // metadata_callback
        ErrorCallback,
        &SELF->decoder_context
    );

    if (!FLAC__stream_decoder_process_until_end_of_metadata(SELF->decoder)) {
        return ErrorCode::DecoderFailedToOpen;
    }

    // Protect WriteCallback
    SELF->decoder_context.buf = nullptr;
    if (!FLAC__stream_decoder_process_single(SELF->decoder)) {
        return ErrorCode::DecoderFailedToOpen;
    }

    SELF->channels = FLAC__stream_decoder_get_channels(SELF->decoder);
    SELF->sample_rate = FLAC__stream_decoder_get_sample_rate(SELF->decoder);
    SELF->samples_per_frame = FLAC__stream_decoder_get_blocksize(SELF->decoder);
    SELF->bits_per_sample = FLAC__stream_decoder_get_bits_per_sample(SELF->decoder);

    SELF->sample_count = FLAC__stream_decoder_get_total_samples(SELF->decoder);
    SELF->duration = SELF->sample_count / FLAC__stream_decoder_get_sample_rate(SELF->decoder) * 1000.f;
    SELF->bit_rate = SELF->sample_rate * 8 * SELF->channels / 1000.f;
    SELF->sample_index = 0;

    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (SELF->decoder) {
        FLAC__stream_decoder_finish(SELF->decoder);
    }
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    SELF->decoder_context.buf = data;
    SELF->decoder_context.buf_used = 0;
    SELF->decoder_context.samples_read = 0;
    const auto ok = FLAC__stream_decoder_process_single(SELF->decoder);
    if (ok) {
        *used = SELF->decoder_context.buf_used;
        *unit_count = SELF->decoder_context.samples_read;
        SELF->sample_index += SELF->decoder_context.samples_read;
    } else {
        *used = 0;
        *unit_count = 0;
        printf("flac DecodeUnit error\n");
    }
    return ErrorCode::Ok;
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    SELF->sample_index = index;
    FLAC__stream_decoder_seek_absolute(SELF->decoder, SELF->sample_index);
    return ErrorCode::Ok;
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    return FLAC__MAX_BLOCK_SIZE * SELF->channels * sizeof(uint32_t);
}

static uint64_t GetUnitIndex(void* ptr) {
    return SELF->sample_index;
}

static uint64_t GetUnitCount(void* ptr) {
    return SELF->sample_count;
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
    return SELF->duration;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

static const char** GetSuffixes(void* ptr) {
    (void) ptr;
    static const char* suffixes[] { "flac", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "flac", nullptr };
    return encodings;
}