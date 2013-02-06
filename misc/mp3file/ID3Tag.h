#ifndef ID3Tag_h
#define ID3Tag_h

#pragma pack(push)
#pragma pack(1)

struct ID3v2Head_t
{
    char header[3];
    char version[1];
    char reversion[1];
    char flag[1];
    char size[4];
};

struct ID3v1Head_t
{
    char header[3];
    char title[30];
    char artist[30];
    char album[30];
    char year[4];
    char comment[28];
    char reserve[1];
    char track[1];
    char genre[1];
};

#pragma pack(pop)

#endif
