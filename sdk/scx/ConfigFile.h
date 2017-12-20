#pragma once

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace scx {

class ConfigFile
{
  public:
    ConfigFile() {}

    ~ConfigFile() {}

    bool Load(const std::string& file, char comment = '#')
    {
        using namespace std;

        Clear();

        ifstream infile;
        infile.open(file.c_str(), ios::in);
        if (!infile.is_open())
            return false;

        stringstream stream;
        stream << infile.rdbuf();
        const string& buf = stream.str();
        infile.close();

        for (size_t off = 0;;) {
            // find line
            size_t lineEnd = buf.find('\n', off);
            if (lineEnd == string::npos) {
                if (off < buf.size())
                    lineEnd = buf.size();
                else
                    break;
            }

            // find comment
            bool iskv = false;
            string key, val;
            for (size_t i = off; i < lineEnd; ++i) {
                if (buf[i] == ' ')
                    continue;
                else if (buf[i] == comment) {
                    iskv = false;
                    break;
                } else {
                    iskv = true;
                    break;
                }
            }

            // append comment/key-val
            if (iskv) {
                // find eq
                size_t eqPos = string::npos;
                for (size_t i = off; i < lineEnd; ++i) {
                    if (buf[i] == '=') {
                        eqPos = i;
                        break;
                    }
                }

                // valid or bad line
                if (eqPos != string::npos) {
                    key = buf.substr(off, eqPos - off);
                    val = buf.substr(eqPos + 1, lineEnd - (eqPos + 1));
                } else {
                    iskv = false;
                    key = buf.substr(off, lineEnd - off);
                }
            } else {
                key = buf.substr(off, lineEnd - off);
            }

            m_Lines.emplace_back(iskv, key, val);
            off = lineEnd + 1;
        }

        m_File = file;
        return true;
    }

    bool Save(std::string file = "") const
    {
        using namespace std;

        if (file.empty()) {
            if (m_File.empty())
                return false;
            else
                file = m_File;
        }

        ofstream outfile;
        outfile.open(file.c_str(), ios::out);
        if (!outfile.is_open())
            return false;

        for (size_t i = 0; i < m_Lines.size(); ++i) {
            outfile.write(m_Lines[i].key.data(), m_Lines[i].key.size());
            if (m_Lines[i].iskv) {
                outfile << '=';
                outfile.write(m_Lines[i].val.data(), m_Lines[i].val.size());
            }
            outfile << '\n';
        }

        outfile.close();

        return true;
    }

    void Clear()
    {
        m_File.clear();
        m_Lines.clear();
        m_Index.clear();
    }

    size_t LineCount() const { return m_Lines.size(); }

    size_t KeyCount() const
    {
        size_t count = 0;
        for (size_t i = 0; i < m_Lines.size(); ++i) {
            if (m_Lines[i].iskv)
                ++count;
        }
        return count;
    }

    bool HasKey(const std::string& key) const
    {
        if (!m_Index.empty()) {
            return m_Index.find(key) != m_Index.end();
        } else {
            for (size_t i = 0; i < m_Lines.size(); ++i) {
                if (m_Lines[i].iskv && m_Lines[i].key == key)
                    return true;
            }
            return false;
        }
    }

    std::vector<std::string> Keys() const
    {
        std::vector<std::string> list;
        list.reserve(m_Lines.size());
        for (size_t i = 0; i < m_Lines.size(); ++i) {
            if (m_Lines[i].iskv)
                list.push_back(m_Lines[i].key);
        }

        return list;
    }

    std::string operator[](const std::string& key) const
    {
        if (!m_Index.empty()) {
            auto iter = m_Index.find(key);
            return iter != m_Index.end() ? m_Lines[iter->second].val : "";
        } else {
            for (size_t i = 0; i < m_Lines.size(); ++i) {
                if (m_Lines[i].iskv && m_Lines[i].key == key)
                    return m_Lines[i].val;
            }
            return "";
        }
    }

    std::string& operator[](const std::string& key)
    {
        std::string* val = nullptr;
        if (!m_Index.empty()) {
            if (m_Index.find(key) != m_Index.end())
                val = &m_Lines[m_Index[key]].val;
        } else {
            for (size_t i = 0; i < m_Lines.size(); ++i) {
                if (m_Lines[i].iskv && m_Lines[i].key == key)
                    val = &m_Lines[i].val;
            }
        }
        if (val == nullptr) {
            m_Lines.emplace_back(true, key);
            val = &(m_Lines[m_Lines.size() - 1].val);
        }
        return *val;
    }

    void Remove(const std::string& key)
    {
        if (!m_Index.empty()) {
            if (m_Index.find(key) != m_Index.end())
                m_Lines.erase(m_Lines.begin() + m_Index[key]);
        } else {
            for (size_t i = 0; i < m_Lines.size(); ++i) {
                if (m_Lines[i].iskv && m_Lines[i].key == key) {
                    m_Lines.erase(m_Lines.begin() + i);
                    break;
                }
            }
        }
        m_Index.clear();
    }

    void AppendComment(const std::string& txt) { m_Lines.emplace_back(false, txt); }

    // Build index can imporve performance when searching key.
    // However, after call Remove(), the index will be cleard.
    // This can be useful when you only want to read/modify key-value.
    void BuildIndex()
    {
        m_Index.clear();
        for (size_t i = 0; i < m_Lines.size(); ++i) {
            if (m_Lines[i].iskv)
                m_Index[m_Lines[i].key] = i;
        }
    }

  private:
    struct Line
    {
        bool iskv;
        std::string key;
        std::string val;

        explicit Line(bool iskv, const std::string& key, const std::string& val = "")
          : iskv(iskv)
          , key(key)
          , val(val)
        {
        }
    };

  private:
    std::string m_File;
    std::vector<Line> m_Lines;
    mutable std::string m_Invaild;

    std::map<std::string, size_t> m_Index;
};
}
