#ifndef SCX_PINYINCOMPARE_HPP
#define SCX_PINYINCOMPARE_HPP

#include <string>

#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <stdio.h>

#ifdef USE_PINYINTABLE_DATA // load pinyin table from external file
#include <fstream>
#else
#include "PinYinTable.hpp"
#endif

namespace scx {

class PinYinCompare
{
private:
    template<size_t S, typename T=void>
    struct Key;

    template<typename T>
    struct Key<2, T>
    {
        typedef unsigned short type;
    };

    template<typename T>
    struct Key<4, T>
    {
        typedef unsigned int type;
    };

    typedef Key<sizeof(wchar_t)>::type key_t;

public:
#ifdef USE_PINYINTABLE_DATA
    bool Init(const std::string& path)
    {
        std::fstream file;
        file.open(path.c_str(), std::ios::in);
        if (!file)
            return false;
        std::string line;
        for (m_count = 0; std::getline(file, line) && m_count < TABLE_SIZE; ++m_count) {
            if (line.size() <= 5)
                continue;
            key_t key = ::strtol(line.substr(0, 4).c_str(), nullptr, 16);
            size_t end = line.find('\t', 5);
            m_table[key] = line.substr(5, end == std::string::npos ? std::string::npos : end-5);
        }
        file.close();
        return (m_count != 0);
    }

    size_t Count() const
    {
        return m_count;
    }
#endif
    /* compare only first character */
    bool CmpUtf8FirstChar(const std::string& a, const std::string& b) const
    {
        wchar_t buf1[2];
        wchar_t buf2[2];
        size_t ret1 = ::mbstowcs(buf1, a.c_str(), 1);
        size_t ret2 = ::mbstowcs(buf2, b.c_str(), 1);

        if (ret1 == (size_t)-1 || ret2 == (size_t)-1)
            return ::strcoll(a.c_str(), b.c_str()) < 0;

        key_t key1 = static_cast<key_t>(buf1[0]);
        key_t key2 = static_cast<key_t>(buf2[0]);
        bool hit1 = key1 < TABLE_SIZE;
        bool hit2 = key2 < TABLE_SIZE;
        const auto& snd1 = hit1 ? m_table[key1] : "";
        const auto& snd2 = hit2 ? m_table[key2] : "";

        if (!hit1 && hit2)
            return true;
        else if (hit1 && !hit2)
            return false;
        else if (!hit1 && !hit2)
            return ::strcoll(a.c_str(), b.c_str()) < 0;
        else
#ifdef USE_PINYINTABLE_DATA
            return ::strcoll(snd1.c_str(), snd2.c_str()) < 0;
#else
            return ::strcoll(snd1, snd2) < 0;
#endif
    }

    bool CmpUtf8(const std::string& a, const std::string& b) const
    {
        const char* bytes1 = a.data();
        const char* bytes2 = b.data();
        int nbytes1 = a.size();
        int nbytes2 = b.size();

        wchar_t buf1[2];
        wchar_t buf2[2];

        while (true) {
            if (nbytes1 <= 0 || nbytes2 <= 0)
                return nbytes1 < nbytes2;

            int ret1 = ::mbtowc(buf1, bytes1, MB_CUR_MAX);
            if (ret1 < 0)
                ::mbtowc(nullptr, nullptr, 0);
            int ret2 = ::mbtowc(buf2, bytes2, MB_CUR_MAX);
            if (ret2 < 0)
                ::mbtowc(nullptr, nullptr, 0);

            if (ret1 <= 0 || ret2 <= 0) {
                break;
            } else if (ret1 == 1 && ret2 == 1) {
                if (bytes1[0] != bytes2[0])
                    return bytes1[0] < bytes2[0];
            } else if (ret1 == 1 && ret2 > 1) {
                return true;
            } else if (ret1 > 1 && ret1 == 1) {
                return false;
            }

            key_t key1 = static_cast<key_t>(buf1[0]);
            key_t key2 = static_cast<key_t>(buf2[0]);

            bool hit1 = key1 < TABLE_SIZE;
            bool hit2 = key2 < TABLE_SIZE;
            if (!hit1 && hit2)
                return true;
            else if (hit1 && !hit2)
                return false;
            else if (!hit1 && !hit2)
                break;
            
            const auto& snd1 = m_table[key1];
            const auto& snd2 = m_table[key2];
#ifdef USE_PINYINTABLE_DATA
            int ret = ::strcoll(snd1.c_str(), snd2.c_str());
#else
            int ret = ::strcoll(snd1, snd2);
#endif
            if (ret != 0)
                return ret < 0;

            bytes1 += ret1;
            nbytes1 -= ret1;
            bytes2 += ret2;
            nbytes2 -= ret2;
        }

        return ::strcoll(bytes1, bytes2) < 0;
    }

private:
#ifdef USE_PINYINTABLE_DATA
    constexpr static const size_t TABLE_SIZE = 0xff00;
    std::string m_table[TABLE_SIZE];
    size_t m_count = 0;
#else
    constexpr static const size_t TABLE_SIZE = scx::PinTableTableSize;
    const char* const* m_table = scx::PinYinTable;
#endif
};

}

#endif
