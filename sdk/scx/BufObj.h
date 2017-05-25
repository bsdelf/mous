#pragma once

#include <assert.h>
#include <inttypes.h>
#include <string.h>

#include <algorithm>
#include <deque>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace scx {

class BufObj
{
  public:
    using stlsize_t = uint32_t;

  public:
    static bool IsBigEndian()
    {
        union
        {
            uint32_t i;
            char c[4];
        } big = { 0x01020304 };
        return (big.c[0] == 0x01);
    }

  public:
    BufObj() = default;

    explicit BufObj(void* b)
      : buf(static_cast<char*>(b))
    {
    }

    void SetBuffer(void* b)
    {
        buf = static_cast<char*>(b);
        off = 0;
    }

    void* Buffer() { return static_cast<void*>(buf); }

    void ResetOffset() { off = 0; }

    uint32_t Offset() const { return off; }

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
    inline BufObj& operator<<(const char* str) { return PutString(str); }

    inline BufObj& operator<<(const std::string& str) { return PutString(str); }

    inline BufObj& operator>>(std::string& str) { return TakeString(str); }

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

    /* queue<POD> */
    template<typename T, typename A>
    BufObj& operator<<(std::queue<T, A> l)
    {
        stlsize_t size = l.size();
        PutRaw(size);
        while (!l.empty()) {
            (*this) << l.front();
            l.pop();
        }
        return *this;
    }

    template<typename T, typename A>
    BufObj& operator>>(std::queue<T, A>& q)
    {
        std::deque<T> t;
        (*this) >> t;
        q = std::queue<T, A>(std::move(t));
        return *this;
    }

    /* stack<POD> */
    template<typename T, typename A>
    BufObj& operator<<(std::stack<T, A> l)
    {
        std::vector<T> t;
        t.reserve(l.size());
        while (!l.empty()) {
            t.push_back(l.top());
            l.pop();
        }
        std::reverse(t.begin(), t.end());
        (*this) << t;
        return *this;
    }

    template<typename T, typename A>
    BufObj& operator>>(std::stack<T, A>& s)
    {
        std::deque<T> t;
        (*this) >> t;
        s = std::stack<T, A>(std::move(t));
        return *this;
    }

    /* set */
    template<typename K, typename C, typename A>
    BufObj& operator<<(const std::set<K, C, A>& s)
    {
        return PutList(s);
    }

    template<typename K, typename C, typename A>
    BufObj& operator>>(std::set<K, C, A>& s)
    {
        stlsize_t size;
        TakeRaw(size);
        for (stlsize_t i = 0; i < size; ++i) {
            K k;
            (*this) >> k;
            s.insert(std::move(k));
        }
        return *this;
    }

    /* map */
    template<typename K, typename T, typename C, typename A>
    BufObj& operator<<(const std::map<K, T, C, A>& m)
    {
        stlsize_t size = m.size();
        PutRaw(size);
        for (const auto& e : m) {
            (*this) << e.first;
            (*this) << e.second;
        }
        return *this;
    }

    template<typename K, typename T, typename C, typename A>
    BufObj& operator>>(std::map<K, T, C, A>& m)
    {
        stlsize_t size;
        TakeRaw(size);
        for (stlsize_t i = 0; i < size; ++i) {
            K k;
            T t;
            (*this) >> k;
            (*this) >> t;
            m[std::move(k)] = std::move(t);
        }
        return *this;
    }

    /*==== specific routines ====*/

    /* string */
    BufObj& PutChars(const char* data, size_t len = (size_t)-1)
    {
        if (len == (size_t)-1) {
            len = strlen(data);
        }
        if (buf != nullptr)
            ::memcpy(buf + off, data, len);
        off += len;
        return *this;
    }

    BufObj& TakeChars(char* data, size_t len)
    {
        if (buf != nullptr)
            ::memcpy(data, buf + off, len);
        off += len;
        return *this;
    }

    BufObj& PutString(const std::string& str)
    {
        stlsize_t size = str.size();
        PutRaw(size);
        if (buf != nullptr)
            ::memcpy(buf + off, str.data(), size);
        off += size;
        return *this;
    }

    BufObj& TakeString(std::string& str)
    {
        stlsize_t size = 0;
        TakeRaw(size);
        if (buf != nullptr)
            str.assign(buf + off, size);
        off += size;
        return *this;
    }

    /* vector, deque */
    template<typename A>
    BufObj& PutArray(const A& a)
    {
        stlsize_t size = a.size();
        PutRaw(size);
        for (stlsize_t i = 0; i < size; ++i) {
            (*this) << a[i];
        }
        return *this;
    }

    template<typename A>
    BufObj& TakeArray(A& a)
    {
        stlsize_t size = 0;
        TakeRaw(size);
        a.resize(size);
        for (stlsize_t i = 0; i < size; ++i) {
            (*this) >> a[i];
        }
        return *this;
    }

    /* list */
    template<typename L>
    BufObj& PutList(const L& l)
    {
        PutRaw((stlsize_t)l.size());
        for (const auto& e : l) {
            (*this) << e;
        }
        return *this;
    }

    template<typename L>
    BufObj& TakeList(L& l)
    {
        stlsize_t size = 0;
        TakeRaw(size);
        l.resize(size);
        for (auto& e : l) {
            (*this) >> e;
        }
        return *this;
    }

    /* POD */
    template<typename T>
    BufObj& PutRaw(T t)
    {
        if (buf != nullptr)
            ::memcpy(buf + off, &t, sizeof(T));
        off += sizeof(T);
        return *this;
    }

    template<typename T>
    BufObj& TakeRaw(T& t)
    {
        if (buf != nullptr)
            ::memcpy(&t, buf + off, sizeof(T));
        else
            ::memset(&t, 0, sizeof(T));
        off += sizeof(T);
        return *this;
    }

    template<typename T>
    const T& Fetch(bool moveNext = false)
    {
        assert(buf != nullptr);
        const T& t = *(T*)(buf + off);
        if (moveNext)
            off += sizeof(T);
        return t;
    }

  private:
    char* buf = nullptr;
    uint32_t off = 0;
};
}
