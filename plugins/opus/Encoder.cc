#include <stdio.h>

#include <plugin/EncoderProto.h>
using namespace mous;

namespace {
    struct Self {
    };
}

static void* Create() {
    return new Self();
}

static void Destroy(void* ptr) {
    delete SELF;
}

void SetChannels(void* ptr, int32_t channels) {
}

static void SetSampleRate(void* ptr, int32_t sampleRate) {
}

static void SetBitsPerSample(void* ptr, int32_t bitsPerSample) {
}

static void SetMediaTag(void* ptr, const MediaTag* tag){
}

static ErrorCode OpenOutput(void* ptr, const char* path)
{
    return ErrorCode::Ok;
}

static void CloseOutput(void* ptr)
{
}

static ErrorCode Encode(void* ptr, const char* data, uint32_t length) {
    return ErrorCode::EncoderFailedToEncode;
}

static ErrorCode Flush(void* ptr) {
    return ErrorCode::EncoderFailedToFlush;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

static const char* GetSuffix(void* ptr) {
    return "opus";
}
