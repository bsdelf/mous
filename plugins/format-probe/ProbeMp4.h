#pragma once

#include <mp4v2/mp4v2.h>

namespace {
    struct MP4File {
        explicit MP4File(const char* path) {
            handle = MP4Read(path);
        }
        ~MP4File() {
            if (handle) {
                MP4Close(handle);
                handle = nullptr;
            }
        }
        operator bool() const {
            return handle;
        }
        MP4FileHandle handle = nullptr;
    };
}

static const char* ProbeMp4(void* ptr, const char* path) {
    MP4File file(path);
    if (!file) {
        return nullptr;
    }
    MP4TrackId trackid = MP4_INVALID_TRACK_ID;
    auto ntrack = MP4GetNumberOfTracks(file.handle);
    for (decltype(ntrack) itrack = 0; itrack < ntrack; ++itrack) {
        trackid = MP4FindTrackId(file.handle, itrack);
        if (trackid != MP4_INVALID_TRACK_ID) {
            const auto type = MP4GetTrackType(file.handle, trackid);
            if (MP4_IS_AUDIO_TRACK_TYPE(type)) {
                break;
            };
        }
    }
    if (trackid == MP4_INVALID_TRACK_ID) {
        return nullptr;
    }
    const char* media_data_name = MP4GetTrackMediaDataName(file.handle, trackid);
    if (strcasecmp(media_data_name, "mp4a") == 0) {
        const auto type = MP4GetTrackEsdsObjectTypeId(file.handle, trackid);
        if (type == MP4_MPEG4_AUDIO_TYPE) {
            return "aac";
        }
    }
    if (strcasecmp(media_data_name, "alac") == 0) {
        return "alac";
    }
    return nullptr;
}