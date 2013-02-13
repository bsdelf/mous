#ifndef SCX_DIR_HPP
#define SCX_DIR_HPP

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

#include <string>
#include <vector>
#include <stack>
#include "FileInfo.hpp"

namespace scx {

class Dir
{
public:
    static bool ChDir(const std::string& path)
    {
        FileInfo file(path);
        if (file.Exists() && file.Type() == FileType::Directory)
            return ::chdir(path.c_str()) == 0;
        else
            return false;
    }

    static bool ChDirUp()
    {
        return ChDir("..");
    }

    static bool Rename(const std::string& oldName, const std::string& newName)
    {
        return rename(oldName.c_str(), newName.c_str()) == 0;
    }

    static bool Remove(const std::string& path)
    {
        return remove(path.c_str()) == 0;
    }

    static bool MakeDir(const std::string& path, mode_t mode)
    {
        using namespace std;

        {
            FileInfo info(path);
            if (info.Exists())
                return info.Type() == FileType::Directory;
        }

        if (mode & 0700)
            mode |= S_IRWXU;

        for (size_t pos = 0; ; ++pos) {
            pos = path.find('/', pos);
            string parent = path.substr(0, pos != string::npos ? pos+1 : path.size());
            //printf("%s\n", parent.c_str());

            FileInfo info(parent);
            if (info.Exists()) {
                if (info.Type() != FileType::Directory)
                    return false;
            } else {
                if (::mkdir(parent.c_str(), mode) != 0) {
                    ::perror(string("MakeDir() " + parent + " failed!").c_str());
                    return false;
                }
            }

            if (pos == string::npos)
                break;
        }

        return true;
    }

    static std::vector<std::string> ListDir(const std::string& path)
    {
        std::vector<std::string> list;
        DIR* dir = ::opendir(path.c_str());
        for (struct dirent* d; (d = ::readdir(dir)) != nullptr; ) {
            list.push_back(d->d_name);
        }
        ::closedir(dir);
        return list;
    }
};

}

#endif
