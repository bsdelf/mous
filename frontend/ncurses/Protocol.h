#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <inttypes.h>

#include <string>
#include <cstdlib>
using namespace std;

#include <scx/BufObj.hpp>
using namespace scx;

//#pragma pack(push, 1)

namespace Protocol {

//==== Helper ====
#define IsVaildEm(Em, v)\
    (v > Em::None && v < Em::Top)

//==== Header ====
const char* const STR_MAGIC = "MOUS";

namespace Group {
enum e 
{
    None = 0,

    App,
    Player,
    Playlist,

    Top
};
}
typedef Group::e EmGroup;

struct Header
{
    char group;
    int32_t payloadSize;

    Header(char _group, int32_t _payloadSize):
        group(_group), payloadSize(_payloadSize)
    {
    }

    static int Size()
    {
        return 4 + sizeof(char) + sizeof(int32_t);
    }

    int TotalSize() const
    {
        return Size() + payloadSize;
    }

    bool Read(char* buf)
    {
        if (std::memcmp(STR_MAGIC, buf, 4) == 0) {
            BufObj(buf+4) >> group >> payloadSize;
            return IsVaildEm(Group, group);
        } else {
            group = Group::None;
            payloadSize = -1;
            return false;
        }
    }

    void Write(char* buf) const
    {
        BufObj(buf).PutChars(STR_MAGIC, 4) << group << payloadSize;
    }
};

namespace Op {

//==== App ====
namespace App {
enum e
{
    None = 0,

    // C:op(char)
    StopService,

    Top
};
}
typedef App::e EmApp;

//==== Player ====
namespace Player {
enum e
{
    None = 0,

    // C:op(char)
    Pause,

    // unimplemented yet
    Seek,

    // C:op(char)
    // S:op(char) percent(char [0, 100])
    GetVolume,

    // C:op(char) percent(char [0, 100])
    SetVolume,

    // C:op(char) running(char 0/1)
    // S:op(char) running(char 0/1)
    Sync,

    // S:op(char) item(MediaItem) sampleRate(int32_t) duration(uint64_t)
    ItemInfo,

    // S:op(char) ms(uint64_t) bitRate(int32_t)
    ItemProgress,

    // For instance:
    // C(running=0) |Sync|  >>  S(running=1)
    // C wait
    // C(running=0=>1)      <<  S(running=1) |ItemInfo|ItemProgress|Sync|
    // C(running=1) |Sync|  >>  S(running=1)
    // C wait
    // C(running=1)         <<  S(running=1) |ItemProgress|Sync|
    // C(running=1) |Sync|  >>  S(running=0)
    // C wait
    // C(running=1=>0)      <<  S(running=0) |Sync|
    //
    // Client should wait for Server's ItemSync before sending *next* ItemSync.

    Top
};
}
typedef Player::e EmPlayer;

//==== Playlist ====
namespace Playlist {
enum e
{
    None = 0,

    // C:op(char) playlist(char)
    // S:op(char) playlist(char)
    Switch,

    // C:op(char) playlist(char) item(int32_t)
    // S:op(char) playlist(char) item(int32_t)
    Select,

    // C:op(char) playlist(char) pos(int32_t)
    // S:op(char) playlist(char) ok(char 0/1)
    Play,   

    // C:op(char) playlist(char) path(string)
    // S:op(char) playlist(char) count(int32_t) item(MediaItem)..*
    Append,

    // C:op(char) playlist(char) pos(int32_t)
    // S:op(char) playlist(char) pos(int32_t)
    Remove,

    // C:op(char) playlist(char)
    // S:op(char) playlist(char)
    Clear,

    // C:op(char) playlist(char)
    // S:{Append..*}
    Sync,

    // C:op(char) playlist1(char) pos1(int32_t) playlist2(char) pos2(int32_t)
    // S:op(char) playlist1(char) pos2(int32_t) playlist2(char) pos2(int32_t)
    Move,

    Top
};
}
typedef Playlist::e EmPlaylist;

}

}

//#pragma pack(pop)

#endif
