#include <vector>
#include <string>
using namespace std;

void DumpMp3Cover(const string& path, vector<char>& buf);
void DumpMp4Cover(const string& path, vector<char>& buf);

bool StoreMp3Cover(const string& path, const char* buf, size_t len);
bool StoreMp4Cover(const string& path, const char* buf, size_t len);
