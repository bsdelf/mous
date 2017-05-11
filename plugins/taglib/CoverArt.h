#include <vector>
#include <string>
using namespace std;

#include <plugin/ITagParser.h>
using namespace mous;

CoverFormat DumpMp3Cover(const string& path, vector<char>& buf);
CoverFormat DumpMp4Cover(const string& path, vector<char>& buf);

bool StoreMp3Cover(const string& path, CoverFormat fmt, const char* buf, size_t len);
bool StoreMp4Cover(const string& path, CoverFormat fmt, const char* buf, size_t len);
