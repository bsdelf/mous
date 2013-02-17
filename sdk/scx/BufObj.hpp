#ifndef SCX_BUFOBJ_HPP
#define SCX_BUFOBJ_HPP

#include <inttypes.h>

#include <cassert>
#include <cstring>
#include <string>
#include <vector>
#include <deque>
#include <queue>
#include <list>
#include <type_traits>

namespace scx {

class BufObj
{
public:
    typedef uint32_t stlsize_t;

private:
    template<typename T>
    struct IsArray {
        enum { value = false };
    };
    template<typename T, typename A>
    struct IsArray<std::vector<T, A>> {
        enum { value = true };
    };
    /*
    template<typename T, typename A>
    struct IsArray<std::deque<T, A>> {
        enum { value = true };
    };
    template<typename T, typename A>
    struct IsArray<std::queue<T, A>> {
        enum { value = true };
    };
    template<typename T, typename A>
    struct IsArray<std::list<T, A>> {
        enum { value = true };
    };
    */
    /*
    template<typename T>
    struct IsArray {
        enum { value = false };
    }
    */

public:
    explicit BufObj(void* _buf = nullptr):
        buf((char*)_buf),
        off(0)
    {
    }

    void SetBuffer(void* _buf)
    {
        buf = (char*)_buf;
        off = 0;
    }

    void* Buffer()
    {
        return (void*)buf;
    }

    void ResetOffset()
    {
        off = 0;
    }

    uint32_t Offset() const
    {
        return off;
    }

    /* POD */
    template<typename T>
    BufObj& operator<<(T t)
    {
        return PutRaw(t);
    }

    template<typename T>
    BufObj& operator>>(T& t)
    {
        return TakeRaw(t);
    }

    /* string */
    template<typename V=void>
    BufObj& operator<<(const char* data)
    {
        return PutChars(data);
    }

    template<typename V=void>
    BufObj& operator<<(const std::string str)
    {
        return PutString(str);
    }

    template<typename V=void>
    BufObj& operator>>(std::string& str)
    {
        return TakeString(str);
    }

    /* vector<POD> */
    template<typename T, typename A>
    BufObj& operator<<(const std::vector<T, A>& t)
    {
        return PutArray(t);
    }

    template<typename T, typename A>
    BufObj& operator>>(std::vector<T, A>& t)
    {
        return TakeArray(t);
    }

    /* deque<POD> */
    template<typename T, typename A>
    BufObj& operator<<(const std::deque<T, A>& t)
    {
        return PutArray(t);
    }

    template<typename T, typename A>
    BufObj& operator>>(std::deque<T, A>& t)
    {
        return TakeArray(t);
    }
    
    /* queue<POD> */
    template<typename T, typename A>
    BufObj& operator<<(const std::queue<T, A>& t)
    {
        return PutList(t);
    }

    template<typename T, typename A>
    BufObj& operator>>(std::queue<T, A>& t)
    {
        return TakeList(t);
    }

    /* list<POD> */
    template<typename T, typename A>
    BufObj& operator<<(const std::list<T, A>& t)
    {
        return PutList(t);
    }

    template<typename T, typename A>
    BufObj& operator>>(std::list<T, A>& t)
    {
        return TakeList(t);
    }

    /* chars */
    BufObj& PutChars(const char* data, size_t len = (size_t)-1)
    {
        if (len == (size_t)-1) {
            len = strlen(data);
        }
        if (buf != nullptr)
            std::memcpy(buf+off, data, len);
        off += len;
        return *this;
    }

    BufObj& TakeChars(char* data, size_t len)
    {
        if (buf != nullptr)
            std::memcpy(data, buf+off, len);
        off += len;
        return *this;
    }

    /* string */
    BufObj& PutString(const std::string& str)
    {
        PutRaw((stlsize_t)str.size());
        if (buf != nullptr)
            std::memcpy(buf+off, str.data(), str.size());
        off += str.size();
        return *this;
    }

    BufObj& TakeString(std::string& str)
    {
        stlsize_t size = 0;
        TakeRaw(size);
        if (buf != nullptr)
            str.assign(buf+off, size);
        off += size;
        return *this;
    }

    /* vector, deque */
    template<typename T>
    BufObj& PutArray(const T& t)
    {
        PutRaw((stlsize_t)t.size());
        for (size_t i = 0; i < t.size(); ++i) {
            (*this) << t[i];
        }
        return *this;
    }

    template<typename T>
    BufObj& TakeArray(T& t)
    {
        stlsize_t size = 0;
        TakeRaw(size);
        t.resize(size);
        for (size_t i = 0; i < size; ++i) {
            (*this) >> t[i];
        }
        return *this;
    }

    /* list, queue */
    template<typename T>
    BufObj& PutList(const T& t)
    {
        typedef typename T::const_iterator iter_t;
        PutRaw((stlsize_t)t.size());
        for (iter_t iter = t.begin(); iter != t.end(); ++iter) {
            (*this) << *iter;
        }
        return *this;
    }

    template<typename T>
    BufObj& TakeList(T& t)
    {
        typedef typename T::iterator iter_t;
        stlsize_t size = 0;
        TakeRaw(size);
        t.resize(size);
        for (iter_t iter = t.begin(); iter != t.end(); ++iter) {
            (*this) >> *iter;
        }
        return *this;
    }

    /* POD */
    template<typename T>
    BufObj& PutRaw(T t)
    {
        if (buf != nullptr)
            std::memcpy(buf+off, &t, sizeof(T));
        off += sizeof(T);
        return *this;
    }

    template<typename T>
    BufObj& TakeRaw(T& t)
    {
        if (buf != nullptr)
            std::memcpy(&t, buf+off, sizeof(T));
        off += sizeof(T);
        return *this;
    }

    template<typename T>
    T& Fetch(bool moveNext = false)
    {
        assert(buf != nullptr);
        T& t = *(T*)(buf+off);
        if (moveNext)
            off += sizeof(T);
        return t;
    }

private:
    char* buf;
    uint32_t off;
};


/*
template<>
inline BufObj& BufObj::operator<<(const char* data)
{
    return PutChars(data);
}

template<>
inline BufObj& BufObj::operator<<(const std::string str)
{
    return PutString(str);
}

template<>
inline BufObj& BufObj::operator>>(std::string& str)
{
    return TakeString(str);
}
*/

}

#endif
