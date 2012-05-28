#ifndef MOUS_PLAYLISTSERIALIZER_H
#define MOUS_PLAYLISTSERIALIZER_H

#include <vector>
#include <fstream>
#include <sstream>

#include <scx/BufObj.hpp>

#include "Playlist.h"

namespace mous {

template<typename item_t>
class PlaylistSerializer
{
private:
    template<typename T>
    struct Trait
    {
        static void New(T& o)
        {
        }

        static T& Ref(T& o)
        {
            return o;
        }

        static const T& Ref(const T& o)
        {
            return o;
        }
    };

    template<typename T>
    struct Trait<T*>
    {
        static void New(T*& o)
        {
            o = new T;
        }

        static T& Ref(T* o)
        {
            return *o;
        }

        static const T& Ref(const T* o)
        {
            return *o;
        }
    };

    typedef Trait<item_t> ItemTrait;

public:
    static bool Load(Playlist<item_t>& list, const std::string file)
    {
        using namespace std;

        bool ret = false;
        fstream infile;
        infile.open(file.c_str(), ios::binary | ios::in);
        if (infile.is_open()) {
            stringstream stream;
            stream << infile.rdbuf();
            const string& str = stream.str();
            ret = Load(list, str.data(), str.size());
        }
        infile.close();
        return ret;
    }

    static bool Load(Playlist<item_t>& list, const char* buf, int len)
    {
        return FromStream(list, buf, len);
    }

    static bool Store(const Playlist<item_t>& list, const std::string file)
    {
        using namespace std;

        bool ret = false;
        fstream outfile;
        vector<char> buffer;

        if (Store(list, buffer)) {
            outfile.open(file.c_str(), ios::binary | ios::out);
            if (outfile.is_open()
                    && (outfile.write(&buffer[0], buffer.size()).tellp() == buffer.size())) {
                ret = true;
            }
            outfile.close();
        }

        return ret;
    }

    static bool Store(const Playlist<item_t>& list, std::vector<char>& outbuf)
    {
        outbuf.resize(ToStream(list, NULL));
        ToStream(list, &outbuf[0]);
        return true;
    }

private:
    static int ToStream(const Playlist<item_t>& list, char* outbuf)
    {
        using namespace scx;

        BufObj buf(outbuf);

        // version
        buf << (int)VERSION;

        // content
        buf << (int)list.m_Mode;

        buf << (int)list.m_ItemQueue.size();
        for (size_t i = 0; i < std::min(STL_MAX, list.m_ItemQueue.size()); ++i) {
            ItemTrait::Ref(list.m_ItemQueue[i]) >> buf;
        }

        buf << list.m_SeqIndex;

        buf << (int)list.m_SeqShuffleQuque.size();
        for (size_t i = 0; i < std::min(STL_MAX, list.m_SeqShuffleQuque.size()); ++i) {
            buf << list.m_SeqShuffleQuque[i];
        }
        
        return buf.Offset();
    }

    
    static bool FromStream(Playlist<item_t>& list, const char* inbuf, int len)
    {
        using namespace scx;

        BufObj buf(const_cast<char*>(inbuf));

        int version;
        buf >> version;

        switch (version) {
            case 1:
                return FromVersion1(list, inbuf + buf.Offset(), len - buf.Offset());

            default:
                break;
        }

        return false;
    }

    static bool FromVersion1(Playlist<item_t>& list, const char* inbuf, int len)
    {
        using namespace scx;

        BufObj buf(const_cast<char*>(inbuf));

        int mode;
        buf >> mode;
        list.m_Mode = (EmPlaylistMode)mode;

        int count;
        buf >> count;
        list.m_ItemQueue.resize(count);
        for (int i = 0; i < count; ++i) {
            ItemTrait::New(list.m_ItemQueue[i]);
            ItemTrait::Ref(list.m_ItemQueue[i]) << buf;
        }

        buf >> list.m_SeqIndex;

        buf >> count;
        list.m_SeqShuffleQuque.resize(count);
        for (int i = 0; i < count; ++i) {
            buf >> list.m_SeqShuffleQuque[i];
        }

        return true;
    }

private:
    static const int VERSION;
    static const size_t STL_MAX;
};

template<typename item_t>
const int PlaylistSerializer<item_t>::VERSION = 1;

template<typename item_t>
const size_t PlaylistSerializer<item_t>::STL_MAX = std::numeric_limits<int>::max();

}

#endif
