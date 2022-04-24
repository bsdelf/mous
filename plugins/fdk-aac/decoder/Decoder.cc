#include <stdio.h>
#include <strings.h>
#include <vector>

#include <fdk-aac/aacdecoder_lib.h>
#include <mp4v2/mp4v2.h>

#include <plugin/DecoderProto.h>
using namespace mous;

namespace {
    struct Self {
        bool is_mp4 = false;

        // mp4v2
        MP4FileHandle mp4file = nullptr;
        int trackid = -1;

        // fdk-aac
        HANDLE_AACDECODER fdk = nullptr;

        // minor
        int32_t bitrate;
        uint64_t duration;

        // meta data
        int32_t channels;
        int64_t timescale;
        int32_t bits;

        // sample info
        uint64_t sampleid;
        uint64_t nsamples;
        std::vector<uint8_t> samplebuff;
    };
}

static void* Create() {
    return new Self;
}

static void Destroy(void* ptr) {
    delete SELF;
}

static ErrorCode OpenMP4(void* ptr, const char* url) {
    SELF->mp4file = MP4Read(url);
    if (!SELF->mp4file) {
        return ErrorCode::DecoderFailedToOpen;
    }

    SELF->trackid = MP4_INVALID_TRACK_ID;
    auto ntrack = MP4GetNumberOfTracks(SELF->mp4file);
    for (decltype(ntrack) itrack = 0; itrack < ntrack; ++itrack) {
        auto trackid = MP4FindTrackId(SELF->mp4file, itrack);
        if (trackid == MP4_INVALID_TRACK_ID) {
            continue;
        }
        if (MP4_IS_AUDIO_TRACK_TYPE(MP4GetTrackType(SELF->mp4file, trackid))) {
            SELF->trackid = trackid;
            break;
        }
    }
    if (SELF->trackid == MP4_INVALID_TRACK_ID) {
        Close(ptr);
        return ErrorCode::DecoderFailedToOpen;
    }

    SELF->bitrate = MP4GetTrackBitRate(SELF->mp4file, SELF->trackid);
    SELF->duration = MP4ConvertFromTrackDuration(
        SELF->mp4file, SELF->trackid, MP4GetTrackDuration(SELF->mp4file, SELF->trackid), MP4_MSECS_TIME_SCALE);

    SELF->channels = MP4GetTrackAudioChannels(SELF->mp4file, SELF->trackid);
    SELF->timescale = MP4GetTrackTimeScale(SELF->mp4file, SELF->trackid);
    SELF->bits = 16;

    SELF->nsamples = MP4GetTrackNumberOfSamples(SELF->mp4file, SELF->trackid);
    SELF->sampleid = 1; // player will call SetUnitIndex() anyway
    SELF->samplebuff.resize(MP4GetTrackMaxSampleSize(SELF->mp4file, SELF->trackid));
    SELF->samplebuff.shrink_to_fit();

    // fdk
    SELF->fdk = aacDecoder_Open(TT_MP4_RAW, 1);
    if (!SELF->fdk) {
        Close(ptr);
        return ErrorCode::DecoderFailedToOpen;
    }

    unsigned char conf[128];
    UINT confBytes = sizeof(conf);
    MP4GetTrackESConfiguration(SELF->mp4file, SELF->trackid, (uint8_t **) &conf, (uint32_t *) &confBytes);
    aacDecoder_ConfigRaw(SELF->fdk, (unsigned char **)&conf, (uint32_t *) &confBytes);

    // see AACDEC_PARAM
    //aacDecoder_SetParam(m_decoder, AAC_PCM_OUTPUT_INTERLEAVED, 1);
    //aacDecoder_SetParam(m_decoder, AAC_PCM_OUTPUT_CHANNELS, m_channels);

    return ErrorCode::Ok;
}

static ErrorCode OpenAAC(void* ptr, const char* url) {
    return ErrorCode::DecoderFailedToOpen;
}

ErrorCode Open(void* ptr, const char* url) {
    // check for mp4 file
    SELF->is_mp4 = false;

    FILE* file = fopen(url, "rb");
    if (!file) {
        return ErrorCode::DecoderFailedToOpen;
    }

    unsigned char header[8];
    size_t nch = fread(header, 1, 8, file);
    fclose(file);
    if (nch != 8) {
        return ErrorCode::DecoderFailedToOpen;
    }

    if (header[4] == 'f' && header[5] == 't' && 
            header[6] == 'y' && header[7] == 'p') {
        SELF->is_mp4 = true;
    }

    return SELF->is_mp4 ? OpenMP4(ptr, url) : OpenAAC(ptr, url);
}

static void Close(void* ptr) {
    if (SELF->mp4file) {
        MP4Close(SELF->mp4file);
        SELF->mp4file = nullptr;
    }
    if (SELF->fdk) {
        aacDecoder_Close(SELF->fdk);
        SELF->fdk = nullptr;
    }
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    /* Frame size of decoded PCM signal
     * 1024 or 960 for AAC-LC
     * 2048 or 1920 for HE-AAC (v2)
     * 512 or 480 for AAC-LD and AAC-ELD
     */
    return 2048 * SELF->channels;
}

static ErrorCode DecodeMp4Unit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    // read sample
    uint8_t* pbytes = SELF->samplebuff.data();
    uint32_t nbytes = SELF->samplebuff.size();
    bool ok = MP4ReadSample(SELF->mp4file, SELF->trackid, SELF->sampleid, &pbytes, &nbytes);
    if (!ok) {
        printf("[fdk-aac] bad sample: %lld\n", SELF->sampleid);
        return ErrorCode::DecoderFailedToRead;
    }
    // decode sample
    UINT valid = nbytes;
    AAC_DECODER_ERROR err = aacDecoder_Fill(SELF->fdk, &pbytes, &nbytes, &valid);
    if (err != AAC_DEC_OK) {
        printf("[fdk-aac] fill failed\n");
        return ErrorCode::DecoderFailedToRead;
    }
    err = aacDecoder_DecodeFrame(SELF->fdk, reinterpret_cast<INT_PCM*>(data), GetMaxBytesPerUnit(ptr), 0);
    if (err == AAC_DEC_NOT_ENOUGH_BITS) {
        printf("[fdk-aac] short frame\n");
        return ErrorCode::DecoderFailedToRead;
    }
    if (err != AAC_DEC_OK) {
        printf("[fdk-aac] bad frame\n");
        return ErrorCode::DecoderFailedToRead;
    }
    // update status
    const auto framesize = aacDecoder_GetStreamInfo(SELF->fdk)->frameSize;
    *used = SELF->channels * framesize * (SELF->bits / 8);
    *unit_count = 1;
    SELF->sampleid += 1;
    return ErrorCode::Ok;
}

static ErrorCode DecodeAacUnit(void* ptr, char* data, uint32_t* used, uint32_t* unitCount) {
    return ErrorCode::DecoderFailedToRead;
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    return SELF->is_mp4 ? DecodeMp4Unit(ptr, data, used, unit_count) : DecodeAacUnit(ptr, data, used, unit_count);
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    SELF->sampleid = index + 1;
    return ErrorCode::Ok;
}

static uint64_t GetUnitIndex(void* ptr) {
    return SELF->sampleid - 1;
}

static uint64_t GetUnitCount(void* ptr) {
    return SELF->nsamples;
}

static AudioMode GetAudioMode(void* ptr) {
    return AudioMode::Stereo;
}

static int32_t GetChannels(void* ptr) {
    return SELF->channels;
}

static int32_t GetBitsPerSample(void* ptr) {
    return SELF->bits;
}

static int32_t GetSampleRate(void* ptr) {
    return SELF->timescale;
}

static int32_t GetBitRate(void* ptr) {
    return SELF->bitrate / 1000;
}

static uint64_t GetDuration(void* ptr) {
    return SELF->duration;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

const char** GetSuffixes(void* ptr) {
    (void) ptr;
    static const char* suffixes[] { "m4a", "mp4", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "aac", nullptr };
    return encodings;
}
