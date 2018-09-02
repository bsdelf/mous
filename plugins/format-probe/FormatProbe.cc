#include <stdio.h>
#include <strings.h>
#include <map>
#include <string>
#include <scx/Conv.h>
#include <scx/FileHelper.h>
using namespace scx;
#include <plugin/FormatProbeProto.h>
using namespace mous;
#ifdef ENABLE_CAF
#include "ProbeCaf.h"
#endif
#ifdef ENABLE_MP4
#include "ProbeMp4.h"
#endif

using ProbeFunc = const char* (*)(void* ptr, const char* path);

namespace {
    struct Self {
        const std::map<std::string, ProbeFunc> probes {
#ifdef ENABLE_CAF
            { "caf", ProbeCaf },
#endif
#ifdef ENABLE_MP4
            { "alac", ProbeMp4 },
            { "m4a", ProbeMp4 },
            { "mp4", ProbeMp4 },
#endif
        };
    };
}

static void* Create(void) {
    return new Self;
}

static void Destroy(void* ptr) {
    delete SELF;
}

static const char* Probe(void* ptr, const char* path) {
    const auto& suffix = ToLower(FileHelper::FileSuffix(path));
    const auto iter = SELF->probes.find(suffix);
    return iter != SELF->probes.end() ? iter->second(ptr, path) : nullptr;
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}

const char** GetSuffixes(void* ptr) {
    (void) ptr;
    static const char* suffixes[] {
        "alac",
#ifdef ENABLE_CAF
        "caf",
#endif
#ifdef ENABLE_MP4
        "m4a",
        "mp4",
#endif
        nullptr
    };
    return suffixes;
}