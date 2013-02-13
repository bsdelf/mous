#ifndef SCX_FILEINFO_HPP
#define SCX_FILEINFO_HPP

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <stdlib.h>

#include <string>

namespace scx {

namespace FileType {
    enum e
    {
        None = 0,
        Regular,
        Directory,
        CharacterSpecial,
        Block,
        Pipe,
        Link,
        Socket,
        Other
    };

    static const char* Name[] = {
        "None",
        "Regular",
        "Directory",
        "Character Special",
        "Block",
        "Pipe/FIFO",
        "Link",
        "Socket",
        "Other"
    };

    static inline std::string ToString(e type)
    {
        return Name[type];
    }

}
typedef FileType::e EmFileType;

class FileInfo
{
public:
    FileInfo():
        m_exists(false),
        m_type(FileType::None)
    {
    }

    explicit FileInfo(const std::string& file):
        m_name(file),
        m_exists(false),
        m_type(FileType::None)
    {
        CheckFileType();
    }
    
    std::string Name() const
    {
        return m_name;
    }

    bool Exists() const
    {
        return m_exists;
    }

    EmFileType Type() const
    {
        return m_type;
    }

    off_t Size() const
    {
        return m_exists ? m_stat.st_size : -1;
    }

    // NOTE: too strict
    bool IsAbs() const
    {
        if (m_name.empty() || !m_exists)
            return false;

        if (m_name[0] != '/')
            return false;

        std::string name(m_name);
        while (name.size() >= 2 && name[name.size()-1] == '/')
            name.erase(name.size()-1, '/');
        return (name == AbsFilePath());
    }

    std::string AbsPath() const
    {
        if (!m_exists || m_name.empty())
            return "";

        std::string name(AbsFilePath());
        name = name.substr(0, name.find_last_of('/'));
        return (!name.empty() ? name : "/");
    }

    std::string AbsFilePath() const
    {
        if (!m_exists)
            return "";

        char buf[PATH_MAX];
        return realpath(m_name.c_str(), buf) != nullptr ? std::string(buf) : "";
    }

    std::string BaseName() const
    {
        if (!m_exists)
            return "";

        std::string name(AbsFilePath());
        size_t pos = name.find_last_of('/');
        return (pos == std::string::npos) ? name : name.substr(pos+1);
    }

    std::string Suffix() const
    {
        if (!m_exists || NotRegularFile())
            return "";

        size_t pos = m_name.find_last_of('.');
        return (pos == std::string::npos) ? "" : m_name.substr(pos+1);
    }

private:
    bool NotRegularFile() const
    {
        if (m_name.empty())
            return true;
        if (m_exists && m_type == FileType::Directory)
            return true;
        if (m_name[m_name.size()-1] == '/')
            return true;
        
        return false;
    }

    void CheckFileType()
    {
        m_exists = !m_name.empty() && (stat(m_name.c_str(), &m_stat) == 0);
        if (!m_exists)
            return;

        if (S_ISREG(m_stat.st_mode))
            m_type = FileType::Regular;
        else if (S_ISDIR(m_stat.st_mode))
            m_type = FileType::Directory;
        else if (S_ISCHR(m_stat.st_mode))
            m_type = FileType::CharacterSpecial;
        else if (S_ISBLK(m_stat.st_mode))
            m_type = FileType::Block;
        else if (S_ISFIFO(m_stat.st_mode))
            m_type = FileType::Pipe;
        else if (S_ISLNK(m_stat.st_mode))
            m_type = FileType::Link;
        else if (S_ISSOCK(m_stat.st_mode))
            m_type = FileType::Socket;
        else
            m_type = FileType::Other;
    }

private:
    struct stat m_stat;

    std::string m_name;
    bool m_exists;
    EmFileType m_type;
};

}

#endif
