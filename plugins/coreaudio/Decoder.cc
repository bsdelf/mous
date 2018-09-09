#include <stdio.h>
#include <string.h>
#include <AudioToolbox/AudioFile.h>
#include <AudioToolbox/AudioConverter.h>
#include <CoreAudio/CoreAudioTypes.h>
#include <CoreServices/CoreServices.h>
#include <plugin/DecoderProto.h>
using namespace mous;

namespace {
    struct Self {
        uint64_t unit_index;
        uint64_t unit_count;

        int32_t channels;
        int32_t bits_per_sample;
        int32_t sample_rate;
        int32_t bit_rate;
        uint64_t duration;

        AudioFileID fileid = nullptr;
        std::vector<char> packet_buffer;
        AudioStreamPacketDescription packet_desc;
        std::vector<char> magic_cookie;
        AudioConverterRef converter_ref = nullptr;
        UInt32 frames_per_unit;
    };
}

static UInt32 MapBitDepth(UInt32 input) {
    return (((input >> 3) + 1) >> 1) << 4;
}

static UInt32 MapSampleRate(UInt32 input) {
    return input > 48000 ? MapSampleRate(input >> 1) : input;
}

static OSStatus AudioFileGetPropertyHelper(
    AudioFileID inAudioFile,
    AudioFilePropertyID inPropertyID,
    void* outPropertyData,
    UInt32* ioPropertyDataSize
) {
    OSStatus status = noErr;
    UInt32 size = 0;
    status = AudioFileGetPropertyInfo(inAudioFile, inPropertyID, &size, nullptr);
    if (status != noErr || size > *ioPropertyDataSize) {
        *ioPropertyDataSize = 0;
        return status;
    }
    status = AudioFileGetProperty(inAudioFile, inPropertyID, &size, outPropertyData);
    if (status != noErr) {
        *ioPropertyDataSize = 0;
        return status;
    }
    *ioPropertyDataSize = size;
    return status;
}

static OSStatus Callback(
    AudioConverterRef inAudioConverter,
    UInt32* ioNumberDataPackets,
    AudioBufferList* ioData,
    AudioStreamPacketDescription** outDataPacketDescription,
    void* inUserData
) {
    OSStatus status = noErr;
    void* ptr = inUserData;
    UInt32 nbytes = SELF->packet_buffer.size();
    if (SELF->unit_index >= SELF->unit_count) {
        return 1;
    }
    // printf("    ------\n");
    // printf("    callback begin %llu/%llu\n", SELF->unit_index, SELF->unit_count);
    // printf("    need %u packets, max size: %lu\n", *ioNumberDataPackets, SELF->packet_buffer.size());
    status = AudioFileReadPacketData(
        SELF->fileid,
        false,
        &nbytes,
        &(SELF->packet_desc),
        SELF->unit_index,
        ioNumberDataPackets,
        SELF->packet_buffer.data()
    );
    // printf("    read %u packets, used size: %u\n", *ioNumberDataPackets, nbytes);
    if (status != noErr) {
        return status;
    }
    ioData->mBuffers[0].mData = SELF->packet_buffer.data();
    ioData->mBuffers[0].mDataByteSize = nbytes;
    if (outDataPacketDescription) {
        *outDataPacketDescription = &(SELF->packet_desc);
    }
    SELF->unit_index += *ioNumberDataPackets;
    return status;
}

static void* Create() {
    return new Self;
}

static void Destroy(void* ptr) {
    delete SELF;
}

static ErrorCode Open(void* ptr, const char* url) {
    OSStatus status = noErr;
    auto cstr = CFStringCreateWithCString(kCFAllocatorDefault, url, kCFStringEncodingUTF8);
    if (!cstr) {
        printf("CFStringCreateWithCString failed\n");
        return ErrorCode::DecoderFailedToOpen;
    }
    auto fsref = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cstr, kCFURLPOSIXPathStyle, false);
    if (!cstr) {
        printf("CFURLCreateWithFileSystemPath failed\n");
        return ErrorCode::DecoderFailedToOpen;
    }
    status = AudioFileOpenURL(fsref, kAudioFileReadPermission, 0, &(SELF->fileid));
    CFRelease(fsref);
    CFRelease(cstr);
    if (status != noErr) {
        return ErrorCode::DecoderFailedToOpen;
    }
    
    UInt32 size;
    printf("----- kAudioFilePropertyDataFormat -----\n");
    AudioStreamBasicDescription srcFormat;
    size = sizeof(srcFormat);
    status = AudioFileGetPropertyHelper(SELF->fileid, kAudioFilePropertyDataFormat, &srcFormat, &size);
    if (status != noErr || size == 0) {
        printf("status: %u, size: %u\n", status, size);
    } else {
        printf("bits: %d\n", srcFormat.mBitsPerChannel);
        printf("channels: %d\n", srcFormat.mChannelsPerFrame);
        printf("sample rate: %.2f\n", srcFormat.mSampleRate);
        printf("bytes per frame: %d\n", srcFormat.mBytesPerFrame);
        printf("format id: %x\n", srcFormat.mFormatID);
        printf("format flags: %d\n", srcFormat.mFormatFlags);
        printf("bytes per frame: %d\n", srcFormat.mBytesPerFrame);
        printf("bytes per packet: %d\n", srcFormat.mBytesPerPacket);
    }
    printf("----- kAudioFilePropertySourceBitDepth -----\n");
    UInt32 srcBitDepth = 0;
    size = sizeof(srcBitDepth);
    status = AudioFileGetPropertyHelper(SELF->fileid, kAudioFilePropertySourceBitDepth, &srcBitDepth, &size);
    if (status != noErr || size == 0) {
        printf("status: %u, size: %u\n", status, size);
    } else {
        printf("%u\n", srcBitDepth);
    }
    printf("----- kAudioFilePropertyInfoDictionary -----\n");
    CFDictionaryRef dictRef;
    size = sizeof(dictRef);
    status = AudioFileGetPropertyHelper(SELF->fileid, kAudioFilePropertyInfoDictionary, &dictRef, &size);
    if (status != noErr || size == 0) {
        printf("status: %u, size: %u\n", status, size);
    } else {
        CFShow(dictRef);
        CFRelease(dictRef);
    }
    printf("----- kAudioFilePropertyMaximumPacketSize -----\n");
    UInt32 maxPacketSize = 0;
    size = sizeof(maxPacketSize);
    status = AudioFileGetPropertyHelper(SELF->fileid, kAudioFilePropertyMaximumPacketSize, &maxPacketSize, &size);
    if (status != noErr || size == 0) {
        printf("status: %u, size: %u\n", status, size);
    } else {
        printf("%u\n", maxPacketSize);
    }
    printf("----- kAudioFilePropertyAudioDataPacketCount -----\n");
    UInt64 dataPacketCount = 0;
    size = sizeof(dataPacketCount);
    status = AudioFileGetPropertyHelper(SELF->fileid, kAudioFilePropertyAudioDataPacketCount, &dataPacketCount, &size);
    if (status != noErr || size == 0) {
        printf("status: %u, size: %u\n", status, size);
    } else {
        printf("%llu\n", dataPacketCount);
    }
    printf("----- kAudioFilePropertyEstimatedDuration -----\n");
    Float64 duration = 0;
    size = sizeof(duration);
    status = AudioFileGetPropertyHelper(SELF->fileid, kAudioFilePropertyEstimatedDuration, &duration, &size);
    if (status != noErr || size == 0) {
        printf("status: %u, size: %u\n", status, size);
    } else {
        printf("%lf\n", duration);
    }
    printf("----- kAudioFilePropertyMagicCookieData -----\n");
    char magicCookie[128];
    UInt32 magicCookieSize = 0;
    size = sizeof(magicCookie);
    status = AudioFileGetPropertyHelper(SELF->fileid, kAudioFilePropertyMagicCookieData, &magicCookie, &size);
    if (status != noErr || size == 0) {
        printf("status: %u, size: %u\n", status, size);
    } else {
        magicCookieSize = size;
        printf("%u\n", magicCookieSize);
    }

    if (maxPacketSize <= 0 || dataPacketCount < 1) {
        return ErrorCode::DecoderFailedToOpen;
    }
    SELF->channels = srcFormat.mChannelsPerFrame;
    SELF->sample_rate = MapSampleRate(srcFormat.mSampleRate);
    SELF->bits_per_sample = srcBitDepth > 0 ? MapBitDepth(srcBitDepth) : 16;
    SELF->frames_per_unit = srcFormat.mFramesPerPacket;
    auto logicMaxPacketSize = maxPacketSize;
    const auto ms_per_frame = SELF->frames_per_unit * 1000 / SELF->sample_rate;
    printf("ms/frame: %u\n", ms_per_frame);
    if (ms_per_frame <= 0) {
        SELF->frames_per_unit = (20 * SELF->sample_rate / 1000) * srcFormat.mFramesPerPacket / srcFormat.mFramesPerPacket;
        logicMaxPacketSize = maxPacketSize * SELF->frames_per_unit;
        printf("frames/unit: %u => %u\n", srcFormat.mFramesPerPacket, SELF->frames_per_unit);
        printf("logicMaxPacketSize: %u => %u\n", maxPacketSize, logicMaxPacketSize);
    }
    SELF->packet_buffer.resize(logicMaxPacketSize);
    SELF->magic_cookie.resize(magicCookieSize);
    if (magicCookieSize > 0) {
        memcpy(SELF->magic_cookie.data(), magicCookie, magicCookieSize);
    }
    SELF->unit_index = 0;
    SELF->unit_count = dataPacketCount;
    SELF->duration = duration * 1000;

    AudioStreamBasicDescription destFormat { 0 };
    destFormat.mFormatID = kAudioFormatLinearPCM;
    destFormat.mFormatFlags = kLinearPCMFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    destFormat.mSampleRate = SELF->sample_rate;
    destFormat.mChannelsPerFrame = srcFormat.mChannelsPerFrame;
    destFormat.mFramesPerPacket = 1;
    destFormat.mBitsPerChannel = SELF->bits_per_sample;
    destFormat.mBytesPerFrame = destFormat.mBitsPerChannel / 8 * srcFormat.mChannelsPerFrame;
    destFormat.mBytesPerPacket = destFormat.mBytesPerFrame * destFormat.mFramesPerPacket;
    status = AudioConverterNew(&srcFormat, &destFormat, &(SELF->converter_ref));
    if (status != noErr) {
        return ErrorCode::DecoderFailedToOpen;
    }
    if (!SELF->magic_cookie.empty()) {
        AudioConverterSetProperty(
            SELF->converter_ref,
            kAudioConverterDecompressionMagicCookie,
            SELF->magic_cookie.size(),
            SELF->magic_cookie.data()
        );
    }
    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (SELF->fileid) {
        AudioFileClose(SELF->fileid);
        SELF->fileid = nullptr;
    }
    if (SELF->converter_ref) {
        AudioConverterDispose(SELF->converter_ref);
        SELF->converter_ref = nullptr;
    }
}

static ErrorCode DecodeUnit(void* ptr, char* data, uint32_t* used, uint32_t* unit_count) {
    if (SELF->unit_index >= SELF->unit_count) {
        *used = 0;
        *unit_count = 0;
        return ErrorCode::DecoderOutOfRange;
    }
    AudioBufferList list { 0 };
    list.mNumberBuffers = 1;
    list.mBuffers[0].mNumberChannels = 2;
    list.mBuffers[0].mDataByteSize = GetMaxBytesPerUnit(ptr);
    list.mBuffers[0].mData = data;
    UInt32 size = SELF->frames_per_unit;
    const auto unit_index = SELF->unit_index;
    UInt32 status = AudioConverterFillComplexBuffer(
        SELF->converter_ref,
        Callback,
        ptr,
        &size,
        &list,
        nullptr
    );
    if (status != noErr && size <= 0) {
        printf("err: %d\n", status);
        *used = 0;
        *unit_count = 0;
        return ErrorCode::DecoderOutOfRange;
    }
    *used = list.mBuffers[0].mDataByteSize;
    *unit_count = SELF->unit_index - unit_index;
    if (*used > 0) {
        const auto duraiton = 1.f * (*used) / (SELF->channels * (SELF->bits_per_sample / 8)) / SELF->sample_rate;
        SELF->bit_rate = SELF->packet_desc.mDataByteSize * 8 / duraiton / 1000;
    } else {
        SELF->bit_rate = 0;
    }
    return ErrorCode::Ok;
}

static ErrorCode SetUnitIndex(void* ptr, uint64_t index) {
    if (index >= SELF->unit_count) {
        return ErrorCode::DecoderOutOfRange;
    }
    SELF->unit_index = index;
    return ErrorCode::Ok;
}

static uint32_t GetMaxBytesPerUnit(void* ptr) {
    return SELF->frames_per_unit * SELF->channels * (SELF->bits_per_sample / 8);
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
    return SELF->duration;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

static const char** GetSuffixes(void* ptr) {
    static const char* suffixes[] { "m4a", "mp4", "aac", "mp3", "caf", "flac", "wav", nullptr };
    return suffixes;
}

static const char** GetEncodings(void* ptr) {
    (void) ptr;
    static const char* encodings[] { "aac", "alac", "mp3", "flac", "lpcm", nullptr };
    return encodings;
}