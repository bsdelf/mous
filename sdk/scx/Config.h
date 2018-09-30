#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <exception>

namespace scx {

class Config
{
  public:
    Config() = default;
    Config(Config&& that) = default;
    Config(const Config& that) = default;

    Config(const std::string& str)
    {
        Concat(str);
    }

    Config(const std::string& str, const std::vector<std::string>& newlines)
    {
        Concat(str, newlines);
    }

    const std::string& GetNewline() const
    {
        return newline_;
    }

    Config& SetNewline(const std::string& newline)
    {
        newline_ = newline;
        return *this;
    }

    const std::string& GetCommentSign() const
    {
        return comment_sign_;
    }

    Config& SetCommentSign(const std::string& sign)
    {
        comment_sign_ = sign;
        return *this;
    }

    const std::string& GetKeyValueDelimiter() const
    {
        return delimiter_;
    }

    Config& SetKeyValueDelimiter(const std::string& delimiter)
    {
        delimiter_ = delimiter;
        return *this;
    }

    size_t CountLines() const
    {
        return lines_.size();
    }

    size_t CountBlanks() const
    {
        return CountLines(LineType::Blank);
    }

    size_t CountComments() const
    {
        return CountLines(LineType::Comment);
    }

    size_t CountKeyValues() const
    {
        return CountLines(LineType::KeyValue);
    }

    size_t CountBytes() const
    {
        size_t n = 0;
        for (size_t i = 0; i < lines_.size(); ++i) {
            const auto& line = lines_[i];
            switch (line.type) {
                case LineType::Blank:
                    break;
                case LineType::Comment:
                    n += comment_sign_.size() + comment_space_.size() + line.items[0].size();
                    break;
                case LineType::KeyValue:
                    n += line.items[0].size() + delimiter_.size() + line.items[1].size();
                    break;
            }
            n += newline_.size();
        }
        return n;
    }

    bool HasKey(const std::string& key) const
    {
        for (size_t i = 0; i < lines_.size(); ++i) {
            const auto& line = lines_[i];
            if (line.type == LineType::KeyValue) {
                if (line.items[0] == key) {
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<std::string> GetKeys() const
    {
        std::vector<std::string> keys;
        for (size_t i = 0; i < lines_.size(); ++i) {
            const auto& line = lines_[i];
            if (line.type == LineType::KeyValue) {
                keys.push_back(line.items[0]);
            }
        }
        return keys;
    }

    const std::string& operator[](const std::string& key) const
    {
        for (size_t i = 0; i < lines_.size(); ++i) {
            const auto& line = lines_[i];
            if (line.type == LineType::KeyValue && line.items[0] == key) {
                return line.items[1];
            }
        }
        throw std::out_of_range("key does not exist");
    }

    std::string& operator[](const std::string& key)
    {
        return const_cast<std::string&>(static_cast<const Config*>(this)->operator[](key));
    }

    Config& Clear()
    {
        lines_.clear();
        return *this;
    }

    Config& RemoveLine(size_t idx)
    {
        lines_.erase(lines_.begin() + idx);
        return *this;
    }

    Config& RemoveKeyValue(const std::string& key)
    {
        lines_.erase(std::remove_if(lines_.begin(), lines_.end(), [&](const Line& line) {
            return line.type == LineType::KeyValue && line.items[0] == key;
        }), lines_.end());
        return *this;
    }

    Config& StripBlanks()
    {
        return StripLines(LineType::Blank);
    }

    Config& StripComments()
    {
        return StripLines(LineType::Comment);
    }

    Config& StripKeyValues()
    {
        return StripLines(LineType::KeyValue);
    }

    Config& Append()
    {
        lines_.push_back({ LineType::Blank, {} });
        return *this;
    }

    Config& Append(const std::string& comment)
    {
        lines_.push_back({ LineType::Comment, { Trim(comment), {} } });
        return *this;
    }

    Config& Append(const std::string& key, const std::string& value)
    {
        lines_.push_back({ LineType::KeyValue, { Trim(key), Trim(value) } });
        return *this;
    }

    Config& Concat(const std::string& str, const std::vector<std::string>& newlines = { "\n", "\r\n" })
    {
        std::vector<Line> lines;
        for (size_t begin = 0; begin < str.size(); ) {
            auto nlat = std::string::npos;
            auto nlsz = 0;
            for (size_t k = 0; k < newlines.size(); ++k) {
                const auto& nl = newlines[k];
                if (nl.empty()) {
                    continue;
                }
                const auto at = str.find(nl, begin);
                if (at < nlat) {
                    nlat = at;
                    nlsz = nl.size();
                }
            }
            if (nlat == std::string::npos) {
                break;
            }
            const auto& line = Trim(str.substr(begin, nlat - begin));
            if (line.empty()) {
                lines.push_back({ LineType::Blank, {} });
            } else if (line.size() >= comment_sign_.size() && line.substr(0, comment_sign_.size()) == comment_sign_) {
                const auto& comment = Trim(line.substr(comment_sign_.size()));
                lines.push_back({ LineType::Comment, { comment } });
            } else {
                const auto pos = line.find(delimiter_);
                if (pos == std::string::npos) {
                    throw std::invalid_argument("key-value delimiter is missing");
                }
                const auto& key = Trim(line.substr(0, pos));
                const auto& value = Trim(line.substr(pos + comment_sign_.size()));
                lines.push_back({ LineType::KeyValue, { key, value } });
            }
            begin = nlat + nlsz;
        }
        if (!lines.empty()) {
            lines_.insert(
                lines_.end(),
                std::make_move_iterator(lines.begin()),
                std::make_move_iterator(lines.end())
            );
        }
        return *this;
    }

    Config& Concat(Config&& that)
    {
        auto& lines = that.lines_;
        if (!lines.empty()) {
            lines_.insert(
                lines_.end(),
                std::make_move_iterator(lines.begin()),
                std::make_move_iterator(lines.end())
            );
        }
        return *this;
    }

    Config& Concat(const Config& that)
    {
        const auto& lines = that.lines_;
        if (!lines.empty()) {
            lines_.insert(lines_.end(), lines.begin(), lines.end());
        }
        return *this;
    }

    std::string ToString() const
    {
        std::string result;
        for (size_t i = 0; i < lines_.size(); ++i) {
            const auto& line = lines_[i];
            switch (line.type) {
                case LineType::Blank:
                    break;
                case LineType::Comment:
                    result += comment_sign_ + comment_space_ + line.items[0];
                    break;
                case LineType::KeyValue:
                    result += line.items[0] + delimiter_ + line.items[1];
                    break;
            }
            result += newline_;
        }
        return result;
    }

  private:
    enum class LineType
    {
        Blank,
        Comment,
        KeyValue
    };

    struct Line
    {
        LineType type;
        std::vector<std::string> items;
    };

    size_t CountLines(LineType type) const
    {
        size_t n = 0;
        for (size_t i = 0; i < lines_.size(); ++i) {
            if (lines_[i].type == type) {
                ++n;
            }
        }
        return n;
    }

    Config& StripLines(LineType type)
    {
        lines_.erase(std::remove_if(lines_.begin(), lines_.end(), [type](const Line& line) {
            return line.type == type;
        }), lines_.end());
        return *this;
    }

    static std::string Trim(const std::string& str, const std::string& chars = " \t")
    {
        const auto first = str.find_first_not_of(chars);
        if (first == std::string::npos) {
            return {};
        }
        const auto last = str.find_last_not_of(chars);
        return str.substr(first, last - first + 1);
    }

  private:
    std::vector<Line> lines_;
    std::string newline_ = "\n";
    std::string delimiter_ = "=";
    std::string comment_sign_ = "#";
    std::string comment_space_ = " ";
};
}
