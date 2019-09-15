#pragma once

#include <util/ErrorCode.h>
#include <util/Option.h>
#include "Interface.h"

static void* Create();
static void Destroy(void* ptr);
static mous::ErrorCode Open(void* ptr, const char* path);
static void Close(void* ptr);
static bool HasTag(void* ptr);
static const char* GetTitle(void* ptr);
static const char* GetArtist(void* ptr);
static const char* GetAlbum(void* ptr);
static const char* GetComment(void* ptr);
static const char* GetGenre(void* ptr);
static int32_t GetYear(void* ptr);
static int32_t GetTrack(void* ptr);
static bool CanEdit(void* ptr);
static bool Save(void* ptr);
static void SetTitle(void* ptr, const char* title);
static void SetArtist(void* ptr, const char* artist);
static void SetAlbum(void* ptr, const char* album);
static void SetComment(void* ptr, const char* comment);
static void SetGenre(void* ptr, const char* genre);
static void SetYear(void* ptr, int32_t year);
static void SetTrack(void* ptr, int32_t track);
static mous::CoverFormat DumpCoverArt(void* ptr, char** out, uint32_t* length);
static bool StoreCoverArt(void* ptr, mous::CoverFormat fmt, const char* data, size_t length);
static bool HasAudioProperties(void* ptr);
static int32_t GetDuration(void* ptr);
static int32_t GetBitRate(void* ptr);
static const mous::BaseOption** GetOptions(void* ptr);
static const char** GetSuffixes(void* ptr);

static const mous::TagParserInterface tag_parser_interface{
    Create,
    Destroy,
    Open,
    Close,
    HasTag,
    GetTitle,
    GetArtist,
    GetAlbum,
    GetComment,
    GetGenre,
    GetYear,
    GetTrack,
    CanEdit,
    Save,
    SetTitle,
    SetArtist,
    SetAlbum,
    SetComment,
    SetGenre,
    SetYear,
    SetTrack,
    DumpCoverArt,
    StoreCoverArt,
    HasAudioProperties,
    GetDuration,
    GetBitRate,
    GetOptions,
    GetSuffixes};

extern "C" {
const mous::TagParserInterface* MousGetTagParserInterface() {
  return &tag_parser_interface;
}
}