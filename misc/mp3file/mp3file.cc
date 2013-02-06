#include <iostream>
#include <string>
#include <vector>
#include <sstream>  // for stringstream
#include <fstream>  // for ifstream/istream
#include <math.h>   // for pow
#include <string.h> // for memset/memcpy
#include <arpa/inet.h> // for msb to lsb
#include <stdlib.h> // for atoi
#include "ID3Tag.h"
#include "APETag.h"

using namespace std;

const char* mpeg_t[] = {
    "mpeg2.5",
    "reserved",
    "mpeg2",
    "mpeg1"
};

const char* layer_t[] = {
    "reserved",
    "layer3",
    "layer2",
    "layer1"
};

const int bps_t[][6] = {
    /*v1,l3   v1,l2   v1,l1   v2,l3  v2,l2  v2,l1  */
    {0,	   0,	 0,    0,   0,   0,  },
    {32,   32,   32,   8,   32,  32, },
    {40,   48,   64,   16,  48,  64, },
    {48,   56,   96,   24,  56,  96, },
    {56,   64,   128,  32,  64,  128,},
    {64,   80,   160,  64,  80,  160,},
    {80,   96,   192,  80,  96,  192,},
    {96,   112,  224,  56,  112, 224,},
    {112,  128,  256,  64,  128, 256,},
    {128,  160,  288,  128, 160, 288,},
    {160,  192,  320,  160, 192, 320,},
    {192,  224,  352,  112, 224, 352,},
    {224,  256,  384,  128, 256, 354,},
    {256,  320,  416,  256, 320, 416,},
    {320,  384,  448,  320, 384, 448,},
    {0,    0,    0,    0,   0,   0,}
};

const char* channel_t[] = {
    "stereo",
    "joint stereo",
    "dual channel",
    "signal channel"
};

const int SampleRate[] = {
    44100,
    48000,
    32000,
        0,
};

#pragma pack(push)
#pragma pack(1)

struct frameheader_t
{
    char sync[11];
    char mpeg[2];
    char layer[2];
    char crc[1];
    char bps[4];
    char rate[2];
    char padding[1];
    char customer[1];
    char channel[2];
    char ext[2];
    char copyright[1];
    char orignal[1];
    char empasis[2];

};

#pragma pack(pop)

int main(int argc, char* argv[])
{
    if (argc <= 1)
        return -1;

    string fileName = argv[1];
    cout << "file name:" << fileName<< endl;

    fstream file;
    streampos length = 0;
    int firstframeBegPos = 0;
    int lastframeEndPos = 0;

    bool hasID3v2 = false;
    bool hasAPETag = false;
    bool hasID3v1 = false;

    file.open(fileName.c_str());
    file.seekg(0, ios::end);
    length = file.tellg();
    file.seekg(0, ios::beg);

    lastframeEndPos = (unsigned int)length-1;
    cout << "file length:" << length << endl;

    // check idv2 head
    ID3v2Head_t id3v2head;
    memset(&id3v2head, 0, sizeof(ID3v2Head_t));

    file.read(id3v2head.header, sizeof(id3v2head.header));
    hasID3v2 = (string(id3v2head.header, sizeof(id3v2head.header)) == "ID3");
    cout << "#" << (string(id3v2head.header, sizeof(id3v2head.header))) << endl; 
    cout << file.tellg() << endl;
    if (hasID3v2)
    {
        cout << "# has id3v2" << endl;
        file.read(id3v2head.version, sizeof(id3v2head.version));
        file.read(id3v2head.reversion, sizeof(id3v2head.reversion));
        file.read(id3v2head.flag, sizeof(id3v2head.flag));
        file.read(id3v2head.size, sizeof(id3v2head.size));

        cout << "ver:" << (int)*id3v2head.version << endl;
        cout << "rev:" << (int)*id3v2head.reversion << endl;
        cout << "flag:" << (int)*id3v2head.flag << endl;

        char* psize = id3v2head.size;
        int id3TotalSize = (int)(id3v2head.size[0]&0x7f) << 21 |
            (int)(id3v2head.size[1]&0x7f) << 14 |
            (int)(id3v2head.size[2]&0x7f) << 7 |
            (int)(id3v2head.size[3]&0x7f) + sizeof(ID3v2Head_t);
        cout << "id3TotalSize:" << id3TotalSize << endl;
        firstframeBegPos = id3TotalSize;
    }
    else
    {
        file.seekg(-3, ios::cur);
    }

    // check idv1 tail
    file.seekg(-(off_t)(sizeof(ID3v1Head_t)), ios::end);
    ID3v1Head_t id3v1tag;
    memset(&id3v1tag, 0, sizeof(ID3v1Head_t));

    file.read(id3v1tag.header, sizeof(id3v1tag.header));
    hasID3v1 = (string(id3v1tag.header, sizeof(id3v1tag.header)) == "TAG");

    if (hasID3v1)
    {
        file.read(id3v1tag.title, sizeof(id3v1tag.title));
        cout << "title:" << id3v1tag.title << endl;

        file.read(id3v1tag.artist, sizeof(id3v1tag.artist));
        cout << "artist:" << id3v1tag.artist << endl;

        fstream tmpfile;
        tmpfile.open("txt", fstream::out);
        tmpfile << id3v1tag.artist;
        tmpfile.close();

        file.read(id3v1tag.album, sizeof(id3v1tag.album));
        cout << "album:" << id3v1tag.album << endl;

        file.read(id3v1tag.year, sizeof(id3v1tag.year));
        cout << "year:" << id3v1tag.year << endl;

        lastframeEndPos -= sizeof(ID3v1Head_t);
    }
    else
    {
        // should be error!
        file.seekg(0, ios::end);
    }

    // check ape tag
    file.seekg(-(off_t)(sizeof(ID3v1Head_t)+sizeof(APETagHead_t)), ios::end);
    APETagHead_t apehead;
    file.read(apehead.header, sizeof(apehead.header));
    if (string(apehead.header, sizeof(apehead.header)) == "APETAGEX")
    {
        file.read((char*)&apehead.version, sizeof(apehead.version));
        file.read((char*)&apehead.size, sizeof(apehead.size));
        file.read((char*)&apehead.items, sizeof(apehead.items));
        file.read((char*)&apehead.flags, sizeof(apehead.flags));
        file.read(apehead.reserved, sizeof(apehead.reserved));

        cout << "ape ver:" << apehead.version << endl;
        cout << "tag size:" << apehead.size << endl;
        cout << "item count:" << apehead.items << endl;
        unsigned int flags = apehead.flags;
        if ((flags>>31) == 0x1)
        {
            cout << "has header" << endl;
            lastframeEndPos -= (apehead.size+sizeof(APETagHead_t));
        }
        else
        {
            lastframeEndPos -= apehead.size;
        }

        if (((flags>>30)&1) == 0)
        {
            cout << "has footer" << endl;
        }
        if (((flags>>29)&1) == 1)
        {
            cout << "is header" << endl;
        }
        else
        {
            cout << "is footer" << endl;
        }
    }
    else
    {
        file.seekg(0, ios::beg);
    }

    // check frames
    unsigned int totalFrames = 0;
    file.seekg(firstframeBegPos, ios::beg);

    unsigned int fmhead;
    bool first = true;

    unsigned short mpeg = -1;
    unsigned short layer = -1;
    bool crc;
    unsigned short bps = -1, bpsi = -1, bpsj = -1;
    unsigned short rate = -1;
    bool padding;
    double duration = 0;
    int bpsCount = 0;

    while (true)
    {
        do 
        {
            // read head
            file.read((char*)&fmhead, sizeof(fmhead));

            fmhead = ntohl(fmhead);
            if ((fmhead & 0xffe00000) == 0xffe00000)
            {
                //cout << ".";
                break;
            }
            else
            {
                cout << "! ";
                cout << layer << ", " << mpeg << ", " << totalFrames << ", ";
                file.seekg(-(file.gcount()-1), ios::cur);
            }
            cout << file.tellg() << endl;
        } 
        while (file.good() && !file.eof() && !file.fail());

        // then clear sync bits
        fmhead = (fmhead<<11) >> 11;

        mpeg = (fmhead>>19);
        layer = (fmhead<<13) >> (13+17);
        crc = ((fmhead<<15) >> (15+16)) == 0 ? true : false;
        bpsi = (fmhead<<16) >> (16+12);
        bpsj = layer + (mpeg==0) ? 0 : 3;
        bps = bps_t[bpsi][bpsj];
        rate = (fmhead<<20) >> (20+10); 

        padding = ((fmhead<<22) >> (22+9)) == 0 ? false : true;

        /*if (first)
          {
          cout << mpeg_t[mpeg] << "," << layer_t[layer] << endl;
        //cout << pbps << endl;
        first = false;
        cout << "padding: " << padding << endl;
        }*/

        ++totalFrames;

        int framesize = 144*bps*1000/SampleRate[rate]+(padding ? 1 : 0);
        if (bps != 0)
        {
            ++bpsCount;
            duration += (double)framesize*8 / (bps*1000.f);
        }
        else
        {
            //cout << bps << ", " << flush;
        }
        // skip body
        file.seekg(framesize-sizeof(fmhead), ios::cur);

        if (file.tellg() >= lastframeEndPos)
            break;
    }

    cout << dec << "firstframeBegPos:" << firstframeBegPos << endl;
    cout << "lastframeEndPos:" << lastframeEndPos << endl;
    cout << "totalframes:" << totalFrames << endl;
    cout << "duration:" << (int)(duration/60) << ":" << int(duration)%60 << endl;
    cout << "bps:" << bps << endl;
    cout << "rate:" << SampleRate[rate] << endl;

    return 0;
}
