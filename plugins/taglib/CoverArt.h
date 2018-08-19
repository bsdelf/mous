#pragma once

#include <vector>
#include <string>
using namespace std;

#include <util/CoverFormat.h>
using namespace mous;

CoverFormat DumpMp3Cover(const string& path, char** out, uint32_t* length);
CoverFormat DumpMp4Cover(const string& path, char** out, uint32_t* length);

bool StoreMp3Cover(const string& path, CoverFormat fmt, const char* buf, size_t len);
bool StoreMp4Cover(const string& path, CoverFormat fmt, const char* buf, size_t len);
