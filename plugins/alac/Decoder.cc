#include <inttypes.h>
#include <strings.h>
#include <stdio.h>
#include <vector>
using namespace std;

#include <mp4v2/mp4v2.h>
#include <alac/codec/ALACDecoder.h>
#include <alac/codec/ALACBitUtilities.h>
#include <plugin/DecoderProto.h>
using namespace mous;

namespace {
    struct Self {
        ALACDecoder decoder;
        MP4FileHandle mp4file = nullptr;
        MP4TrackId trackid = MP4_INVALID_TRACK_ID;
        int32_t bitrate;
        uint64_t duration;
        int32_t channels;
        int64_t timescale;
        int32_t bits;
        uint64_t sampleid;
        uint64_t nsamples;
        std::vector<uint8_t> samplebuff;
    };
}

static void* Create() {
    auto self = new Self();
    return self;
}

static void Destroy(void* ptr) {
    delete SELF;
}

static ErrorCode Open(void* ptr, const char* url) {
    SELF->mp4file = MP4Read(url);
    if (SELF->mp4file == nullptr) {
        return ErrorCode::DecoderFailedToOpen;
    }

    // NOTE: the following weird unlogical code is learned from "mp4v2-1.9.1/src/mp4info.cpp"
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

    const char* media_data_name = MP4GetTrackMediaDataName(SELF->mp4file, SELF->trackid);
    if (strncasecmp("alac", media_data_name, 5) != 0) {
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

    // TODO: use designated initialize in c++20
    ALACSpecificConfig config;
    config.frameLength = kALACDefaultFramesPerPacket;
    config.compatibleVersion = 0;
    config.bitDepth = static_cast<uint8_t>(SELF->bits);
    config.pb = 40;
    config.mb = 10;
    config.kb = 14;
    config.numChannels = static_cast<uint8_t>(SELF->channels);
    config.maxRun = 255;
    config.maxFrameBytes = 0;
    config.avgBitRate = 0;
    config.sampleRate = static_cast<uint32_t>(SELF->timescale);
    int32_t status = SELF->decoder.Init(&config, sizeof(config));
    if (status != ALAC_noErr) {
        return ErrorCode::DecoderFailedToOpen;
    }
    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (SELF->mp4file != nullptr) {
        MP4Close(SELF->mp4file);
        SELF->mp4file = nullptr;
    }
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    // read encoded data
    uint8_t* pbytes = SELF->samplebuff.data();
    uint32_t nbytes = SELF->samplebuff.size();
    const bool ok = MP4ReadSample(SELF->mp4file, SELF->trackid, SELF->sampleid, &pbytes, &nbytes);
    if (!ok) {
        printf("mp4 bad sample: %lld\n", SELF->sampleid);
        return ErrorCode::DecoderFailedToRead;
    }
    // feed to decoder
    BitBuffer bits;
    bits.cur = pbytes;
    bits.end = pbytes + nbytes;
    bits.bitIndex = 0;
    bits.byteSize = nbytes;
    uint32_t samples = 0;
    const int32_t status = SELF->decoder.Decode(&bits, reinterpret_cast<uint8_t*>(data), kALACDefaultFramesPerPacket, SELF->channels, &samples);
    if (status != ALAC_noErr) {
        printf("alac failed to decode: %d\n", status);
        return ErrorCode::DecoderFailedToRead;
    }
    // update status
    *used = SELF->channels * (SELF->bits / 8) * samples;
    *unit_count = 1;
    SELF->sampleid += 1;
    return ErrorCode::Ok;
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    SELF->sampleid = index + 1;
    return ErrorCode::Ok;
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    return SELF->channels * sizeof(uint32_t) * kALACDefaultFramesPerPacket;
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

static const char** GetSuffixes(void* ptr) {
    (void) ptr;
    static const char* suffixes[] { "m4a", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "alac", nullptr };
    return encodings;
}