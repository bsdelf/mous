#pragma once

#include <inttypes.h>
#include <string>

namespace mous {

struct MediaTag {
  std::string title;
  std::string artist;
  std::string album;
  std::string comment;
  std::string genre;
  int32_t year = -1;
  int32_t track = -1;

  template <typename buf_t>
  void operator>>(buf_t& buf) const {
    buf << title << artist << album << comment << genre << year << track;
  }

  template <typename buf_t>
  void operator<<(buf_t& buf) {
    buf >> title >> artist >> album >> comment >> genre >> year >> track;
  }
};

}  // namespace mous
