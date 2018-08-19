#include "CoverArt.h"

#include <string.h>

#include <scx/Conv.h>
using namespace scx;

//#include <iostream>

#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>

#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>
using namespace TagLib;

static CoverFormat DumpID3v2Cover(ID3v2::Tag* idtag, char** out, uint32_t* length);
static bool StoreID3v2Cover(ID3v2::Tag* idtag, CoverFormat fmt, const char* buf, size_t len);

/*======== dump ========*/
CoverFormat DumpMp3Cover(const string& path, char** out, uint32_t* length) {
    TagLib::MPEG::File file(path.c_str(), false);
    return DumpID3v2Cover(file.ID3v2Tag(), out, length);
}

CoverFormat DumpMp4Cover(const string& path, char** out, uint32_t* length) {
    CoverFormat format = CoverFormat::None;

    TagLib::MP4::File file(path.c_str(), false);
    MP4::Tag* mp4tag = file.tag();

    if (!mp4tag) {
        //cout << "no mp4 tag found!" << endl;
        return format;
    }
    MP4::ItemListMap::Iterator iter = mp4tag->itemListMap().find("covr");
    if (iter == mp4tag->itemListMap().end()) {
        //cout << "\"covr\" not found!" << endl;
        return format;
    }   
    MP4::CoverArtList list = iter->second.toCoverArtList();
    if (list.isEmpty()) {
        //cout << "no cover art!" << endl;
        return format;
    }

    //cout << "CoverArtList count: " << list.size() << endl;
    //cout << "type: " << list[0].format() << endl;
    switch (list[0].format()) {
        case MP4::CoverArt::JPEG: {
            format = CoverFormat::JPEG;
            break;
        }
        case MP4::CoverArt::PNG: {
            format = CoverFormat::PNG;
            break;
        }
        default: {
            format = CoverFormat::None;
            break;
        }
    }
    const ByteVector& v = list[0].data();
    if (v.size() == 0) {
        return format;
    }
    *out = new char[v.size()];
    *length = v.size();
    memcpy(*out, v.data(), v.size());
    return format;
}

static CoverFormat DumpID3v2Cover(ID3v2::Tag* idtag, char** out, uint32_t* length) {
    CoverFormat format = CoverFormat::None;

    if (!idtag) {
        //cout << "no id3v2 tag found!" << endl;
        return format;
    } 

    const char* PIC_ID[] { "APIC", "PIC" };
    for (const char* ID: PIC_ID) {
        ID3v2::FrameList frameList = idtag->frameList(ID);
        if (frameList.isEmpty()) {
            continue;
        }

        for (auto iter = frameList.begin(); iter != frameList.end(); ++iter) {
            auto frame = static_cast<ID3v2::AttachedPictureFrame*>(*iter);
            string mime = ToLower(frame->mimeType().to8Bit());
            cout << "type: " << (int) frame->type() << endl;
            cout << "mime: " << mime << endl;
            const ByteVector& v = frame->picture();
            if (v.size() != 0) {
                *out = new char[v.size()];
                *length = v.size();
                memcpy(*out, v.data(), v.size());
                if (mime.find("jpeg") != string::npos) {
                    format = CoverFormat::JPEG;
                } else if (mime.find("png") != string::npos){
                    format = CoverFormat::PNG;
                }
                return format;
            }
        }
    }

    return format;
}

/*======== store ========*/
bool StoreMp3Cover(const string& path, CoverFormat fmt, const char* buf, size_t len) {
    TagLib::MPEG::File file(path.c_str(), false);
    if (StoreID3v2Cover(file.ID3v2Tag(), fmt, buf, len)) {
        auto ref = dynamic_cast<TagLib::MPEG::File*>(&file);
        if (ref != nullptr) {
            return ref->save(TagLib::MPEG::File::TagTypes::ID3v2, true, 3, true);
        } else {
            cout << "bad type" << endl;
        }
    }
    return false;
}

bool StoreMp4Cover(const string& path, CoverFormat fmt, const char* buf, size_t len) {
    TagLib::MP4::File file(path.c_str(), false);
    MP4::Tag* mp4tag = file.tag();
    if (mp4tag == nullptr) {
        //cout << "no mp4 tag found!" << endl;
        return false;
    }

    // prepare taglib param
    MP4::CoverArt::Format tformat;
    switch (fmt) {
        case CoverFormat::JPEG:
            tformat = MP4::CoverArt::JPEG;
            break;

        case CoverFormat::PNG:
            tformat = MP4::CoverArt::PNG;
            break;

        default:
            return false;
    }
    ByteVector tdata(buf, len);
    MP4::CoverArt art(tformat, tdata);
    MP4::CoverArtList list;
    list.append(art);
    MP4::Item item(list);

    // update
    MP4::ItemListMap::Iterator iter = mp4tag->itemListMap().find("covr");
    if (iter == mp4tag->itemListMap().end()) {
        //cout << "insert \"covr\"" << endl;
        mp4tag->itemListMap().insert("covr", item);
    } else {
        //cout << "update \"covr\"" << endl;
        iter->second = item;
    }

    return mp4tag->save();
}

static bool StoreID3v2Cover(ID3v2::Tag* idtag, CoverFormat fmt, const char* buf, size_t len) {
    if (idtag == nullptr) {
        //cout << "no id3v2 tag found!" << endl;
        return false;
    }

    // prepare taglib data
    String mime;
    switch (fmt) {
        case CoverFormat::JPEG:
            mime = "image/jpeg";
            break;

        case CoverFormat::PNG:
            mime = "image/png";
            break;

        default:
            return false;
    }
    ByteVector imgData(buf, len);

    ID3v2::AttachedPictureFrame* frame = nullptr;

    // remove previous FrontCover
    const char* PIC_ID[] = { "APIC", "PIC" };
    for (int i = 0; i < 2; ++i) {
        const ID3v2::FrameList frameList = idtag->frameListMap()[PIC_ID[i]];

        ID3v2::FrameList::ConstIterator iter = frameList.begin();
        for (; iter != frameList.end(); ++iter) {
            frame = static_cast<ID3v2::AttachedPictureFrame*>(*iter);
            if (frame->type() == ID3v2::AttachedPictureFrame::FrontCover) {
                idtag->removeFrame(frame, true);
                //cout << "remove" << endl;
            }
        }
    }

    // insert new FrontCover
    frame = new ID3v2::AttachedPictureFrame;
    frame->setMimeType(mime);
    frame->setPicture(imgData);
    frame->setType(ID3v2::AttachedPictureFrame::FrontCover);
    idtag->addFrame(frame);

    return true;
}
