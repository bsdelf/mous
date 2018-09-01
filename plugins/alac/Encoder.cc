#include <fstream>
#include <cstring>
using namespace std;

#include <plugin/EncoderProto.h>
using namespace mous;

namespace {
    struct Self {
    };
}

static void* Create() {
    auto self = new Self;
    return self;
}

static void Destroy(void* ptr) {
    delete SELF;
}

void SetChannels(void* ptr, int32_t channels) {
}

void SetSampleRate(void* ptr, int32_t sample_rate) {
}

static void SetBitsPerSample(void* ptr, int32_t bits_per_sample) {
}

static void SetMediaTag(void* ptr, const MediaTag* tag) {

}

static ErrorCode OpenOutput(void* ptr, const char* path) {
    return ErrorCode::Ok;
}

static void CloseOutput(void* ptr) {

}

static ErrorCode Encode(void* ptr, const char* data, uint32_t length) {
    return ErrorCode::Ok;
}

static ErrorCode Flush(void* ptr) {
    return ErrorCode::Ok;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

static const char* GetSuffix(void* ptr) {
    return "caf";
}
