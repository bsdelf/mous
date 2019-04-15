#pragma once

#include <algorithm>
#include <string>
#include <vector>
#include <exception>

namespace scx {

class Config {
public:
    struct Blank {};

    struct Comment {
        Comment(std::string comment): comment(std::move(comment)) {}
        std::string comment;
    };

    struct KeyValue {
        KeyValue(std::string key, std::string value): key(std::move(key)), value(std::move(value)) {}
        std::string key;
        std::string value;
    };

private:
  struct Line {
      enum Type {
          kBlank,
          kComment,
          kKeyValue,
      };

      Line(Blank): type(kBlank) {}
      Line(Comment line): type(kComment), items({std::move(line.comment)}) {}
      Line(KeyValue line): type(kKeyValue), items({std::move(line.key), std::move(line.value)}) {}

      Type type;
      std::vector<std::string> items;
  };

public:
    Config() = default;
    Config(Config&& that) = default;
    Config(const Config& that) = default;

    Config(const std::string& str, const std::vector<std::string>& newlines = { "\n", "\r\n" }) {
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
                lines.emplace_back(Blank());
            } else if (line.size() >= comment_sign_.size() && line.substr(0, comment_sign_.size()) == comment_sign_) {
                auto comment = Trim(line.substr(comment_sign_.size()));
                lines.emplace_back(Comment(std::move(comment)));
            } else {
                const auto pos = line.find(delimiter_);
                if (pos == std::string::npos) {
                    throw std::invalid_argument("key-value delimiter is missing");
                }
                auto key = Trim(line.substr(0, pos));
                auto value = Trim(line.substr(pos + comment_sign_.size()));
                lines.emplace_back(KeyValue(std::move(key), std::move(value)));
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
    }

    Config(Blank line) {
        lines_.emplace_back(std::move(line));
    }

    Config(Comment line) {
        lines_.emplace_back(std::move(line));
    }

    Config(KeyValue line) {
        lines_.emplace_back(std::move(line));
    }

    template<typename... Args>
    Config& Concat(Args&&... args) {
        Config that(std::forward<Args>(args)...);
        if (!that.lines_.empty()) {
            lines_.insert(
                lines_.end(),
                std::make_move_iterator(that.lines_.begin()),
                std::make_move_iterator(that.lines_.end())
            );
        }
        return *this;
    }

    const std::string& Newline() const {
        return newline_;
    }

    Config& SetNewline(const std::string& newline) {
        newline_ = newline;
        return *this;
    }

    const std::string& CommentSign() const {
        return comment_sign_;
    }

    Config& SetCommentSign(const std::string& sign) {
        comment_sign_ = sign;
        return *this;
    }

    const std::string& KeyValueDelimiter() const {
        return delimiter_;
    }

    Config& SetKeyValueDelimiter(const std::string& delimiter) {
        delimiter_ = delimiter;
        return *this;
    }

    size_t CountLines() const {
        return lines_.size();
    }

    size_t CountBlanks() const {
        return CountLines(Line::kBlank);
    }

    size_t CountComments() const {
        return CountLines(Line::kComment);
    }

    size_t CountKeyValues() const {
        return CountLines(Line::kKeyValue);
    }

    size_t CountBytes() const {
        size_t n = 0;
        for (size_t i = 0; i < lines_.size(); ++i) {
            const auto& line = lines_[i];
            switch (line.type) {
                case Line::kBlank:
                    break;
                case Line::kComment:
                    n += comment_sign_.size() + comment_space_.size() + line.items[0].size();
                    break;
                case Line::kKeyValue:
                    n += line.items[0].size() + delimiter_.size() + line.items[1].size();
                    break;
            }
            n += newline_.size();
        }
        return n;
    }

    bool HasKey(const std::string& key) const {
        for (size_t i = 0; i < lines_.size(); ++i) {
            const auto& line = lines_[i];
            if (line.type == Line::kKeyValue) {
                if (line.items[0] == key) {
                    return true;
                }
            }
        }
        return false;
    }

    std::vector<std::string> Keys() const {
        std::vector<std::string> keys;
        for (size_t i = 0; i < lines_.size(); ++i) {
            const auto& line = lines_[i];
            if (line.type == Line::kKeyValue) {
                keys.push_back(line.items[0]);
            }
        }
        return keys;
    }

    const std::string& operator[](const std::string& key) const {
        for (size_t i = 0; i < lines_.size(); ++i) {
            const auto& line = lines_[i];
            if (line.type == Line::kKeyValue && line.items[0] == key) {
                return line.items[1];
            }
        }
        throw std::out_of_range("key does not exist");
    }

    std::string& operator[](const std::string& key) {
        return const_cast<std::string&>(static_cast<const Config*>(this)->operator[](key));
    }

    Config& Clear() {
        lines_.clear();
        return *this;
    }

    Config& RemoveLine(size_t idx) {
        lines_.erase(lines_.begin() + idx);
        return *this;
    }

    Config& RemoveKeyValue(const std::string& key) {
        lines_.erase(std::remove_if(lines_.begin(), lines_.end(), [&](const Line& line) {
            return line.type == Line::kKeyValue && line.items[0] == key;
        }), lines_.end());
        return *this;
    }

    Config& StripBlanks() {
        return StripLines(Line::kBlank);
    }

    Config& StripComments() {
        return StripLines(Line::kComment);
    }

    Config& StripKeyValues() {
        return StripLines(Line::kKeyValue);
    }

    std::string ToString() const {
        std::string result;
        for (size_t i = 0; i < lines_.size(); ++i) {
            const auto& line = lines_[i];
            switch (line.type) {
                case Line::kBlank:
                    break;
                case Line::kComment:
                    result += comment_sign_ + comment_space_ + line.items[0];
                    break;
                case Line::kKeyValue:
                    result += line.items[0] + delimiter_ + line.items[1];
                    break;
            }
            result += newline_;
        }
        return result;
    }

    size_t CountLines(Line::Type type) const {
        size_t n = 0;
        for (size_t i = 0; i < lines_.size(); ++i) {
            if (lines_[i].type == type) {
                ++n;
            }
        }
        return n;
    }

    Config& StripLines(Line::Type type) {
        lines_.erase(std::remove_if(lines_.begin(), lines_.end(), [type](const Line& line) {
            return line.type == type;
        }), lines_.end());
        return *this;
    }

    static std::string Trim(const std::string& str, const std::string& chars = " \t") {
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
