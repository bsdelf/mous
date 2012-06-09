#include "CoverArt.h"

#include <scx/Conv.hpp>
using namespace scx;

#include <iostream>

#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>

#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>
using namespace TagLib;

static EmCoverFormat DumpID3v2Cover(ID3v2::Tag* mp3Tag, vector<char>& buf);

EmCoverFormat DumpMp3Cover(const string& path, vector<char>& buf)
{
    TagLib::MPEG::File file(path.c_str());
    return DumpID3v2Cover(file.ID3v2Tag(), buf);
}

static EmCoverFormat DumpID3v2Cover(ID3v2::Tag* mp3Tag, vector<char>& buf)
{
    EmCoverFormat format = CoverFormat::None;

    if (mp3Tag == NULL) {
        cout << "no id3v2 tag found!" << endl;
        return format;
    } 

    ID3v2::FrameList frameList;
    ID3v2::AttachedPictureFrame* frame;

    const char* picId[] = { "APIC", "PIC" };
    for (int i = 0; i < 2; ++i) {
        frameList = mp3Tag->frameListMap()[picId[i]];
        if (!frameList.isEmpty()) {
            ID3v2::FrameList::ConstIterator iter = frameList.begin();
            for (; iter != frameList.end(); ++iter) {
                frame = static_cast<ID3v2::AttachedPictureFrame*>(*iter);
                string mime = ToLower(frame->mimeType().to8Bit());

                cout << "type: " << (int) frame->type() << endl;
                cout << "mime: " << mime << endl;

                if (mime.find("jpeg") != string::npos) {
                    format = CoverFormat::JPEG;
                } else if (mime.find("png") != string::npos){
                    format = CoverFormat::PNG;
                }

                const ByteVector& v = frame->picture();
                if (v.size() != 0) {
                    buf.resize(v.size());
                    memcpy(&buf[0], v.data(), v.size());
                }

                return format;
            }
        } else {
            cout << picId[i] << " not found!" << endl;
        }
    }

    return format;
}

EmCoverFormat DumpMp4Cover(const string& path, vector<char>& buf)
{
    EmCoverFormat format = CoverFormat::None;

    TagLib::MP4::File file(path.c_str());
    MP4::Tag* mp4tag = file.tag();

    if (mp4tag == NULL) {
        cout << "no mp4 tag found!" << endl;
        return format;
    }

    MP4::ItemListMap::Iterator iter = mp4tag->itemListMap().find("covr");
    if (iter != mp4tag->itemListMap().end()) {
        MP4::CoverArtList list = iter->second.toCoverArtList();
        if (list.isEmpty()) {
            cout << "no cover art!" << endl;
        }

        cout << "CoverArtList count: " << list.size() << endl;
        cout << "type: " << list[0].format() << endl;

        switch (list[0].format()) {
            case MP4::CoverArt::JPEG:
                format = CoverFormat::JPEG;
                break;

            case MP4::CoverArt::PNG:
                format = CoverFormat::PNG;
                break;

            default:
                format = CoverFormat::None;
                break;
        }

        const ByteVector& v = list[0].data();
        if (v.size() == 0)
            return format;

        buf.resize(v.size());
        memcpy(&buf[0], v.data(), v.size());
    } else {
        cout << "\"covr\" not found!" << endl;
    }

    return format;
}

bool StoreMp3Cover(const string& path, EmCoverFormat fmt, const char* buf, size_t len)
{
    return false;
}

bool StoreMp4Cover(const string& path, EmCoverFormat fmt, const char* buf, size_t len)
{
    return false;
}
