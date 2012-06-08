#include <map>
#include <string>
#include <fstream>
#include <iostream>

#include <taglib/flacfile.h>
#include <taglib/flacpicture.h>
#include <taglib/xiphcomment.h>

#include <taglib/mp4file.h>
#include <taglib/mp4tag.h>
#include <taglib/mp4coverart.h>

#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/id3v2frame.h>
#include <taglib/attachedpictureframe.h>

#include <taglib/apefile.h>
#include <taglib/apetag.h>
#include <taglib/apeitem.h>
#include <taglib/tmap.h>

using namespace std;
using namespace TagLib;

typedef void (*FnDump)(const string&);

void DumpMP4(const string& filename)
{
    TagLib::MP4::File file(filename.c_str());
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

        ofstream imgFile;
        imgFile.open("img", ios::binary | ios::out);
        imgFile.write(list[0].data().data(), list[0].data().size());
        cout << imgFile.tellp()/1024.f << " KiB dumped" << endl;
        imgFile.close();

    } else {
        cout << "\"covr\" not found!" << endl;
    }
}

void DumpID3v2(ID3v2::Tag* mp3Tag)
{
    if (mp3Tag == NULL){
        cout << "no id3v2 tag found!" << endl;
        return;
    } 

    ID3v2::FrameList frameList;
    ID3v2::AttachedPictureFrame* frame;

    const char* picId[] = { "APIC", "PIC" };
    char count = '0';
    for (int i = 0; i < 2; ++i) {
        frameList = mp3Tag->frameListMap()[picId[i]];
        if (!frameList.isEmpty()) {
            ID3v2::FrameList::ConstIterator iter = frameList.begin();
            for (; iter != frameList.end(); ++iter) {
                frame = static_cast<ID3v2::AttachedPictureFrame*>(*iter);
                cout << "type: " << (int) frame->type() << endl;
                cout << "mime: " << frame->mimeType().toCString() << endl;

                ofstream imgFile;
                imgFile.open((string("img") + string(&(++count), 1)).c_str(), ios::binary | ios::out);
                imgFile.write(frame->picture().data(), frame->picture().size());
                cout << imgFile.tellp()/1024.f << " KiB dumped" << endl;
                imgFile.close();
            }
        } else {
            cout << picId[i] << " not found!" << endl;
        }
    }
}

void DumpMP3(const string& filename)
{
    TagLib::MPEG::File file(filename.c_str());
    DumpID3v2(file.ID3v2Tag());
}


void DumpFlac(const string& filename)
{
    FLAC::File file(filename.c_str());
    List<FLAC::Picture*> picList = file.pictureList();
    if (picList.isEmpty()) {
        cout << "no flac pic found!" << endl;
        ID3v2::Tag* tag = file.ID3v2Tag();
        if (tag != NULL) {
            DumpID3v2(tag);
        } else {
            cout << "no id3v2 found!" << endl;
            Ogg::XiphComment* com = file.xiphComment();
            if (com == NULL) {
                cout << "no xiph found!" << endl;
                return;
            }
            //Ogg::FieldListMap::ConstIterator iter = com->fieldListMap().find("METADATA_BLOCK_PICTURE");
            //Ogg::FieldListMap::ConstIterator iter = com->fieldListMap().begin();
            //while (iter++ != com->fieldListMap().end()) {
                //cout << iter->second.size()() << endl;
            //}
            //if (iter == com->fieldListMap().end()) {
            //    cout << "no xiph img" << endl;
            //}
        }
    } else {
        for (unsigned int i = 0; i < picList.size(); ++i) {
            cout << picList[i]->mimeType().toCString() << endl;
        }
    }
}

void DumpApe(const string& filename)
{
    APE::File file(filename.c_str());
    APE::Tag* tag = file.APETag(false);
    const APE::ItemListMap& map = tag->itemListMap();
    for (APE::ItemListMap::ConstIterator iter = map.begin();
            iter != map.end(); ++iter) {
        cout << "name:" << iter->first.to8Bit() << endl;
    }
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        return 0;
    }

    string path(argv[1]);
    cout << "input file: " << path << endl;
    string ext(path.substr(path.rfind('.')+1));
    cout << "file ext: " << ext << endl;

    map<string, FnDump> dumpers;
    dumpers["m4a"] = &DumpMP4;
    dumpers["mp3"] = &DumpMP3;
    dumpers["flac"] = &DumpFlac;
    dumpers["ape"] = &DumpApe;
    
    if (dumpers.find(ext) != dumpers.end()) {
        dumpers[ext](path);
    } else {
        cout << "unsupported audio format!" << endl;
    }

    return 0;
}
