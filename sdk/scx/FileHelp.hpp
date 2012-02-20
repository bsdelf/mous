#ifndef SCX_FILEHELP_HPP
#define SCX_FILEHELP_HPP

#include <inttypes.h>
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

static inline bool CreateFile(const char* path, uint64_t size)
{
    ofstream stream(path, ios::out);
    stream.seekp(size-1, ios::beg);
    stream.put('\0');
    stream.close();
    return (FileSize(path) == size) ? true : false;
}

static inline string FileSuffix(const string& name, char ch = '.')
{
    size_t pos = name.find_last_of(ch);
    return name.substr(pos+1, name.size());
}

static inline string FileDir(const string& path)
{
    size_t pos = path.find_last_of('/');
    if (pos == string::npos) {
	return "./";
    } else if (pos == path.size()-1){
	return path.substr(0, path.size());
    } else {
	return path.substr(0, pos+1);
    }
}

}

#endif
