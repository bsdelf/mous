#pragma once

#include <stdio.h>
#include <iostream>
#include <vector>

#include <fdk-aac/aacdecoder_lib.h>
#include <mp4v2/mp4v2.h>

#include <plugin/DecoderProto.h>
using namespace mous;

struct Self {
    bool is_mp4 = false;

    // mp4v2
    MP4FileHandle mp4 = nullptr;
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

static void* Create() {
    return new Self;
}

static void Destroy(void* ptr) {
    delete SELF;
}

static ErrorCode OpenMP4(void* ptr, const char* url) {
    // mp4v2
    SELF->mp4 = MP4Read(url);
    if (SELF->mp4 == nullptr) {
        return ErrorCode::DecoderFailedToOpen;
    }

    {
        // get first audio track
        SELF->trackid = 0;
        int ntrack = MP4GetNumberOfTracks(SELF->mp4);
        for (int itrack = 1; itrack <= ntrack; ++itrack) {
            if (MP4_IS_AUDIO_TRACK_TYPE(MP4GetTrackType(SELF->mp4, itrack))) {
                SELF->trackid = itrack;
                std::cout << SELF->trackid << std::endl;
                break;
            }
        }
        if (SELF->trackid == 0) {
            Close(ptr);
            return ErrorCode::DecoderFailedToOpen;
        }
    }

    SELF->bitrate = MP4GetTrackBitRate(SELF->mp4, SELF->trackid);
    SELF->duration = MP4ConvertFromTrackDuration(
        SELF->mp4, SELF->trackid, MP4GetTrackDuration(SELF->mp4, SELF->trackid), MP4_MSECS_TIME_SCALE);

    SELF->channels = MP4GetTrackAudioChannels(SELF->mp4, SELF->trackid);
    SELF->timescale = MP4GetTrackTimeScale(SELF->mp4, SELF->trackid);
    SELF->bits = 16;

    SELF->nsamples = MP4GetTrackNumberOfSamples(SELF->mp4, SELF->trackid);
    SELF->sampleid = 1; // player will call SetUnitIndex() anyway
    SELF->samplebuff.resize(MP4GetTrackMaxSampleSize(SELF->mp4, SELF->trackid));
    SELF->samplebuff.shrink_to_fit();

    //cout << "info: " << m_bitrate << ", "<< m_channels << ", "<< m_nsamples << ", " << m_timescale << endl;

    // fdk
    SELF->fdk = aacDecoder_Open(TT_MP4_RAW, 1);
    if (SELF->fdk == nullptr) {
        Close(ptr);
        return ErrorCode::DecoderFailedToOpen;
    }

    unsigned char conf[128];
    UINT confBytes = sizeof(conf);
    MP4GetTrackESConfiguration(SELF->mp4, SELF->trackid, (uint8_t **) &conf, (uint32_t *) &confBytes);
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
    if (file == nullptr) {
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
    if (SELF->mp4 != nullptr) {
        MP4Close(SELF->mp4);
        SELF->mp4 = nullptr;
    }
    if (SELF->fdk != nullptr) {
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
    bool ok = MP4ReadSample(SELF->mp4, SELF->trackid, SELF->sampleid, &pbytes, &nbytes);
    if (!ok) {
        std::cout << "mp4 bad sample: " << SELF->sampleid << std::endl;
        return ErrorCode::DecoderFailedToRead;
    }
    //cout << nbytes << " - " << m_samplebuff.size() << endl;

    // decode sample
    UINT valid = nbytes;
    AAC_DECODER_ERROR err = aacDecoder_Fill(SELF->fdk, &pbytes, &nbytes, &valid);
    if (err != AAC_DEC_OK) {
        std::cout << "fdk bad fill" << std::endl;
        return ErrorCode::DecoderFailedToRead;
    }

    err = aacDecoder_DecodeFrame(SELF->fdk, reinterpret_cast<INT_PCM*>(data), GetMaxBytesPerUnit(ptr), 0);
    if (err == AAC_DEC_NOT_ENOUGH_BITS) {
        std::cout << "fdk short frame" << std::endl;
        return ErrorCode::DecoderFailedToRead;
    }
    if (err != AAC_DEC_OK) {
        std::cout << "fdk bad frame" << std::endl;
        return ErrorCode::DecoderFailedToRead;
    }

    auto framesize = aacDecoder_GetStreamInfo(SELF->fdk)->frameSize;
    *used = SELF->channels * framesize * (SELF->bits / 8);
    *unit_count = 1;

    // move forward
    SELF->sampleid += 1;

    return ErrorCode::Ok;
}

static ErrorCode DecodeAacUnit(void* ptr, char* data, uint32_t* used, uint32_t* unitCount) {
    return ErrorCode::Ok;
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
