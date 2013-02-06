#ifndef APETag_h
#define APETag_h

#pragma pack(push)
#pragma pack(1)

struct APETagHead_t
{
    char header[8];
    int version;
    int size;
    int items;
    int flags;
    char reserved[8];
};

#pragma pack(pop)

#endif
