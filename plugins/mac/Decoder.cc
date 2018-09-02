#include <cstdio>
#include <cwctype>
#include <MAC_SDK/Source/Shared/All.h>
#include <MAC_SDK/Source/Shared/NoWindows.h>
#include <MAC_SDK/Source/Shared/CharacterHelper.h>
#include <MAC_SDK/Source/MACLib/APEDecompress.h>

#include <plugin/DecoderProto.h>
using namespace mous;

namespace {
    struct Self {
        APE::IAPEDecompress* decompress = nullptr;

        uint64_t block_index;
        uint64_t block_count;

        uint32_t block_align;
        uint32_t blocks_per_frame;
        uint32_t blocks_per_read;

        int32_t channels;
        int32_t bits_per_sample;
        int32_t sample_rate;
        int32_t bit_rate;
        uint64_t duration;
    };
}

static void* Create() {
    return new Self;
}

static void Destroy(void* ptr) {
    Close(ptr);
    delete SELF;
}

static ErrorCode Open(void* ptr, const char* url) {
    int err = ERROR_SUCCESS;

    APE::CSmartPtr<wchar_t> spFileName = APE::CAPECharacterHelper::GetUTF16FromANSI(url);
    SELF->decompress = CreateIAPEDecompress(spFileName, &err);

    if (SELF->decompress == nullptr || err != ERROR_SUCCESS) {
        return ErrorCode::DecoderFailedToOpen;
    }

    SELF->channels = SELF->decompress->GetInfo(APE::APE_INFO_CHANNELS);
    SELF->sample_rate = SELF->decompress->GetInfo(APE::APE_INFO_SAMPLE_RATE);
    SELF->bits_per_sample = SELF->decompress->GetInfo(APE::APE_INFO_BITS_PER_SAMPLE);

    SELF->duration = SELF->decompress->GetInfo(APE::APE_INFO_LENGTH_MS);

    SELF->block_align = SELF->decompress->GetInfo(APE::APE_INFO_BLOCK_ALIGN);
    SELF->blocks_per_frame = SELF->decompress->GetInfo(APE::APE_INFO_BLOCKS_PER_FRAME);
    SELF->block_count = SELF->decompress->GetInfo(APE::APE_INFO_TOTAL_BLOCKS);
    SELF->blocks_per_read = SELF->blocks_per_frame / 16;

    SELF->block_index = 0;

    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (SELF->decompress) {
        delete SELF->decompress; 
        SELF->decompress = nullptr;
    }
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    if (SELF->block_index < SELF->block_count) {
        SELF->bit_rate = SELF->decompress->GetInfo(APE::APE_DECOMPRESS_CURRENT_BITRATE);

        APE::intn blocksRecv = 0;
        auto ret = SELF->decompress->GetData(data, SELF->blocks_per_read, &blocksRecv);
        switch (ret) {
            case ERROR_SUCCESS:
            {
                *used = blocksRecv * SELF->block_align;
                *unit_count = blocksRecv;
                SELF->block_index += blocksRecv;
                return ErrorCode::Ok;
            }

            case ERROR_INVALID_CHECKSUM: {
                printf("FATAL: mac invalid checksum!\n");
                break;
            }

            default: {
                printf("FATAL: mac bad unit!\n");
                break;
            }
        }
    } 

    printf("FATAL: mac hit end or error occured!\n");

    *used = 0;
    *unit_count = SELF->block_count;
    SELF->block_index = SELF->block_count;

    return ErrorCode::DecoderOutOfRange;
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    SELF->decompress->Seek(index);
    SELF->block_index = index;
    return ErrorCode::Ok;
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    return SELF->blocks_per_read * SELF->block_align;
}

static uint64_t GetUnitIndex(void* ptr) {
    return SELF->block_index;
}

static uint64_t GetUnitCount(void* ptr) {
    return SELF->block_count;
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
    static const char* suffixes[] { "ape", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "ape", nullptr };
    return encodings;
}