#pragma once

#include <util/ErrorCode.h>
#include <util/Option.h>
#include "Interface.h"

static void* Create();
static void Destroy(void* ptr);
static mous::ErrorCode Open(void* ptr);
static void Close(void* ptr);
static mous::ErrorCode Setup(void* ptr, int32_t* channels, int32_t* sample_rate, int32_t* bits_per_sample);
static mous::ErrorCode Write(void* ptr, const char* data, uint32_t length);
static int GetVolume(void* ptr);
static void SetVolume(void* ptr, int avg);
static const mous::BaseOption** GetOptions(void* ptr);

static const mous::OutputInterface output_interface {
    Create,
    Destroy,
    Open,
    Close,
    Setup,
    Write,
    GetVolume,
    SetVolume,
    GetOptions
};

extern "C" {
    const mous::OutputInterface* MousGetOutputInterface() {
        return &output_interface;
    }
}