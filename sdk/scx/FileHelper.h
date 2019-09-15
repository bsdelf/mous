#pragma once

#include <inttypes.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>

namespace scx {

namespace FileHelper {

inline size_t
FileSize(const char* path) {
  struct stat statbuf;
  stat(path, &statbuf);
  return statbuf.st_size;
}

inline bool
CreateFile(const char* path, uint64_t size) {
  using namespace std;
  ofstream stream(path, ios::out);
  stream.seekp(size - 1, ios::beg);
  stream.put('\0');
  stream.close();
  return (FileSize(path) == size) ? true : false;
}

inline std::string
FileSuffix(const std::string& name, char ch = '.') {
  size_t pos = name.find_last_of(ch);
  return name.substr(pos + 1, name.size());
}

inline std::string
FileDir(const std::string& path) {
  using namespace std;
  size_t pos = path.find_last_of('/');
  if (pos == string::npos) {
    return "./";
  } else if (pos == path.size() - 1) {
    return path.substr(0, path.size());
  } else {
    return path.substr(0, pos + 1);
  }
}

inline std::string
ReadAll(const char* filename, bool asBin = false) {
  using namespace std;

  ifstream instream;
  if (asBin) {
    instream.open(filename, fstream::in | fstream::binary);
  } else {
    instream.open(filename, fstream::in);
  }

  stringstream stream;
  if (instream.is_open()) {
    stream << instream.rdbuf();
  }

  instream.close();

  return stream.str();
}
}  // namespace FileHelper
}  // namespace scx
