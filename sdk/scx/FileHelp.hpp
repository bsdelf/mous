#ifndef SCX_FILEHELP_HPP
#define SCX_FILEHELP_HPP

#include <sys/types.h>
#include <sys/stat.h>
#include <fstream>

using namespace std;

namespace scx
{
static inline size_t FileSize(const char* path)
{
    struct stat statbuf;
    stat(path, &statbuf);
    return statbuf.st_size;
}

static inline bool CreateFile(const char* path, long size)
{
    ofstream stream(path, ios::out);
    stream.seekp(size-1, ios::beg);
    stream.put('\0');
    stream.close();
    return (FileSize(path) == size) ? true : false;
}

static inline string GetNameSuffix(const string& name, char ch = '.')
{
    size_t pos = name.find_last_of(ch);
    return name.substr(pos+1, foo.size());
}

}

#endif
