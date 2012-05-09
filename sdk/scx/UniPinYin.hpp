#ifndef SCX_UNIPINYIN_HPP
#define SCX_UNIPINYIN_HPP

#include <string>
#include <map>
#include <fstream>

#include <locale.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>

namespace scx {

class UniPinYin
{
public:
    bool LoadMap(const std::string& path)
    {
        using namespace std;
        key_t key;
        string line;
        fstream file;
        file.open(path.c_str(), ios::in);
        while (getline(file, line)) {
            if (line.size() <= 5)
                continue;
            key = strtol(line.substr(0, 4).c_str(), NULL, 16);
            size_t end = line.find('\t', 5);
            m_Map[key] = line.substr(5, end == string::npos ? string::npos : end-5);
        }
        file.close();
        return !m_Map.empty();
    }

    void UnloadMap()
    {
        m_Map.clear();
    }

    bool Utf8StrCmp(const std::string& a, const std::string& b) const
    {
        using namespace std;

        wchar_t buf1[2];
        wchar_t buf2[2];
        size_t ret1 = mbstowcs(buf1, a.c_str(), 1);
        size_t ret2 = mbstowcs(buf2, b.c_str(), 1);

        if (ret1 == (size_t)-1 || ret2 == (size_t)-1)
            return strcoll(a.c_str(), b.c_str()) < 0;

        bool shortWChar = sizeof(wchar_t) == sizeof(key_t);

        map_iter_t iter1 = m_Map.find(shortWChar ? *(key_t*)buf1 : *(key_long_t*)buf1);
        const char* data1 = ((iter1 == m_Map.end()) ? a : iter1->second).c_str();

        map_iter_t iter2 = m_Map.find(shortWChar ? *(key_t*)buf2 : *(key_long_t*)buf2);
        const char* data2 = ((iter2 == m_Map.end()) ? b : iter2->second).c_str();
        //cout << "cmp:" << a << "(" << data1 << ")-" << b << "(" << data2 << ")"<< endl;

        if (iter1 == m_Map.end() && iter2 != m_Map.end())
            return true;
        else if (iter1 != m_Map.end() && iter2 == m_Map.end())
            return false;
        else
            return strcoll(data1, data2) < 0;
    }

private:
    typedef unsigned short key_t;
    typedef unsigned int key_long_t;

    typedef std::map<key_t, std::string> map_t;
    typedef map_t::const_iterator map_iter_t;
    map_t m_Map;
};

}

#endif
