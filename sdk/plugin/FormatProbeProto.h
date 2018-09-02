#pragma once

#include <unistd.h>
#include <inttypes.h>
#include <util/AudioMode.h>
#include <util/ErrorCode.h>
#include <util/Option.h>
#include "Interface.h"

static void* Create();
static void Destroy(void* ptr);
static const char* Probe(void* ptr, const char* path);
static const mous::BaseOption** GetOptions(void* ptr);
static const char** GetSuffixes(void* ptr);

static mous::FormatProbeInterface format_probe_interface {
    Create,
    Destroy,
    Probe,
    GetOptions,
    GetSuffixes
};

extern "C" {
    const mous::FormatProbeInterface* MousGetFormatProbeInterface() {
        return &format_probe_interface;
    }
}
