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
        m_Exists(false),
        m_Type(FileType::None)
    {
    }

    ~FileInfo()
    {
    }

    explicit FileInfo(const std::string& file):
        m_Name(file),
        m_Exists(false),
        m_Type(FileType::None)
    {
        CheckFileType();
    }
    
    std::string Name() const
    {
        return m_Name;
    }

    bool Exists() const
    {
        return m_Exists;
    }

    EmFileType Type() const
    {
        return m_Type;
    }

    off_t Size() const
    {
        return m_Exists ? m_Stat.st_size : -1;
    }

    // NOTE: too strict
    bool IsAbs() const
    {
        if (m_Name.empty() || !m_Exists)
            return false;

        if (m_Name[0] != '/')
            return false;

        std::string name(m_Name);
        while (name.size() >= 2 && name[name.size()-1] == '/')
            name.erase(name.size()-1, '/');
        return name == AbsFilePath();
    }

    std::string AbsPath() const
    {
        if (!m_Exists || m_Name.empty())
            return "";

        std::string name(AbsFilePath());
        name = name.substr(0, name.find_last_of('/'));
        return !name.empty() ? name : "/";
    }

    std::string AbsFilePath() const
    {
        if (!m_Exists)
            return "";

        char buf[PATH_MAX];
        return realpath(m_Name.c_str(), buf) != NULL ? std::string(buf) : "";
    }

    std::string BaseName() const
    {
        if (!m_Exists)
            return "";

        std::string name(AbsFilePath());
        size_t pos = name.find_last_of('/');
        return (pos == std::string::npos) ? name : name.substr(pos+1);
    }

    std::string Suffix() const
    {
        if (!m_Exists || NotRegularFile())
            return "";

        size_t pos = m_Name.find_last_of('.');
        return (pos == std::string::npos) ? "" : m_Name.substr(pos+1);
    }

private:
    bool NotRegularFile() const
    {
        if (m_Name.empty())
            return true;
        if (m_Exists && m_Type == FileType::Directory)
            return true;
        if (m_Name[m_Name.size()-1] == '/')
            return true;
        
        return false;
    }

    void CheckFileType()
    {
        m_Exists = !m_Name.empty() && (stat(m_Name.c_str(), &m_Stat) == 0);
        if (!m_Exists)
            return;

        if (S_ISREG(m_Stat.st_mode))
            m_Type = FileType::Regular;
        else if (S_ISDIR(m_Stat.st_mode))
            m_Type = FileType::Directory;
        else if (S_ISCHR(m_Stat.st_mode))
            m_Type = FileType::CharacterSpecial;
        else if (S_ISBLK(m_Stat.st_mode))
            m_Type = FileType::Block;
        else if (S_ISFIFO(m_Stat.st_mode))
            m_Type = FileType::Pipe;
        else if (S_ISLNK(m_Stat.st_mode))
            m_Type = FileType::Link;
        else if (S_ISSOCK(m_Stat.st_mode))
            m_Type = FileType::Socket;
        else
            m_Type = FileType::Other;
    }

private:
    struct stat m_Stat;

    std::string m_Name;
    bool m_Exists;
    EmFileType m_Type;
};

}

#endif
