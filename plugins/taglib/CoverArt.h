#include <vector>
#include <string>
using namespace std;

#include <plugin/ITagParser.h>
using namespace mous;

EmCoverFormat DumpMp3Cover(const string& path, vector<char>& buf);
EmCoverFormat DumpMp4Cover(const string& path, vector<char>& buf);

bool StoreMp3Cover(const string& path, EmCoverFormat fmt, const char* buf, size_t len);
bool StoreMp4Cover(const string& path, EmCoverFormat fmt, const char* buf, size_t len);
