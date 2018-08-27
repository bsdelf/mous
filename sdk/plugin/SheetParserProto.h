#pragma once

#include <deque>
#include <util/ErrorCode.h>
#include <util/Option.h>
#include <util/MediaItem.h>
#include "Interface.h"

static void* Create();
static void Destroy(void* ptr);
static void DumpFile(void* ptr, const char* path, std::deque<mous::MediaItem>* list);
static void DumpStream(void* ptr, const char* stream, std::deque<mous::MediaItem>* list);
static const mous::BaseOption** GetOptions(void* ptr);
static const char** GetSuffixes(void* ptr);

static const mous::SheetParserInterface sheet_parser_interface {
    Create,
    Destroy,
    DumpFile,
    DumpStream,
    GetOptions,
    GetSuffixes
};

extern "C" {
    const mous::SheetParserInterface* MousGetSheetParserInterface() {
        return &sheet_parser_interface;
    }
}