#include "CoverArt.h"

#include <iostream>

#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>

#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>
using namespace TagLib;

static void DumpID3v2Cover(ID3v2::Tag* mp3Tag, vector<char>& buf);

void DumpMp3Cover(const string& path, vector<char>& buf)
{
    TagLib::MPEG::File file(path.c_str());
    DumpID3v2Cover(file.ID3v2Tag(), buf);
}

static void DumpID3v2Cover(ID3v2::Tag* mp3Tag, vector<char>& buf)
{
    if (mp3Tag == NULL) {
        cout << "no id3v2 tag found!" << endl;
        return;
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
                cout << "type: " << (int) frame->type() << endl;
                cout << "mime: " << frame->mimeType().to8Bit() << endl;

                const ByteVector& v = frame->picture();
                if (v.size() == 0)
                    return;
                buf.resize(v.size());
                memcpy(&buf[0], v.data(), v.size());

                return;
            }
        } else {
            cout << picId[i] << " not found!" << endl;
        }
    }
}

void DumpMp4Cover(const string& path, vector<char>& buf)
{
    TagLib::MP4::File file(path.c_str());
    MP4::Tag* mp4tag = file.tag();

    if (mp4tag == NULL) {
        cout << "no mp4 tag found!" << endl;
        return;
    }

    MP4::ItemListMap::Iterator iter = mp4tag->itemListMap().find("covr");
    if (iter != mp4tag->itemListMap().end()) {
        MP4::CoverArtList list = iter->second.toCoverArtList();
        if (list.isEmpty()) {
            cout << "no cover art!" << endl;
        }
        cout << "CoverArtList count: " << list.size() << endl;

        cout << "type: " << list[0].format() << endl;

        const ByteVector& v = list[0].data();
        if (v.size() == 0)
            return;
        buf.resize(v.size());
        memcpy(&buf[0], v.data(), v.size());

        return;
    } else {
        cout << "\"covr\" not found!" << endl;
    }
}

bool StoreMp3Cover(const string& path, const char* buf, size_t len)
{
    return false;
}

bool StoreMp4Cover(const string& path, const char* buf, size_t len)
{
    return false;
}
