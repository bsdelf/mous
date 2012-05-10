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

namespace Op {
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
}

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
            return IsVaildEm(Op::Group, group);
        } else {
            group = Op::Group::None;
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

    // req:op(char)
    StopService,
    // req:op(char) path(str); ret:op(char)
    LoadPlay,

    Top
};
}
typedef App::e EmApp;

//==== Player ====
namespace Player {
enum e
{
    None = 0,

    Play,   
    Seek,
    Stop,
    Pause,
    Resume,
    // req:op(char); ret:op(char) ms(uint64_t)
    TotalMs,
    // req:op(char); ret:op(char) ms(uint64_t)
    CurrentMs,

    Top
};
}
typedef Player::e EmPlayer;

//==== Playlist ====
// op(char) 
namespace Playlist {
enum e
{
    None = 0,

    Open,
    Close,
    New,
    Delete,
    Rename,
    Add,
    Remove,
    Move,

    Top
};
}
typedef Playlist::e EmPlaylist;

}

}

//#pragma pack(pop)

#endif
