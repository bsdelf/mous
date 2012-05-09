#ifndef SCX_DIR_HPP
#define SCX_DIR_HPP

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#include <string>
#include <vector>
#include "FileInfo.hpp"

namespace scx {

class Dir
{
public:
    Dir()
    {
    }

    ~Dir()
    {
    }

    explicit Dir(const std::string& path):
        m_Path(path)
    {
        Refresh();
    }

    bool ChDir(const std::string& path)
    {
        FileInfo file(path);
        if (file.Exists() && file.Type() == FileType::Directory) {
            if (chdir(path.c_str()) == 0) {
                m_Path = path;
                return true;
            }
        } 
        Refresh();
        return false;
    }

    bool ChDirUp()
    {
        return ChDir("..");
    }

    size_t EntryCount() const
    {
        return m_Files.size();
    }

    std::vector<std::string> Entries() const
    {
        return m_Files;
    }

    std::string Path() const
    {
        return m_Path;
    }

    bool MkDir(const std::string& path, int mode) const
    {
        return mkdir(path.c_str(), mode) == 0;
    }

    bool IsRoot()
    {
        return m_Path == "/";
    }

    bool Exists()
    {
        return FileInfo(m_Path).Exists();
    }

    bool Rename(const std::string& newName)
    {
        bool ret = rename(m_Path.c_str(), newName.c_str());
        if (ret)
            m_Path = newName;
        return ret;
    }

    bool Remove()
    {
        bool ret = remove(m_Path.c_str()) == 0;
        if (ret)
            m_Files.clear();
        return ret;
    }

    std::vector<std::string> Refresh()
    {
        m_Files = ListDir(m_Path);
        return m_Files;
    }

public:
    static inline std::vector<std::string> ListDir(const std::string& path)
    {
        std::vector<std::string> list;
        DIR* dir = opendir(path.c_str());
        for (struct dirent* d; (d = readdir(dir)) != NULL; ) {
            list.push_back(d->d_name);
        }
        closedir(dir);
        return list;
    }

private:
    std::string m_Path;
    std::vector<std::string> m_Files;
};

}

#endif
