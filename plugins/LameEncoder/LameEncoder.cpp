#include "LameEncoder.h"
using namespace std;

LameEncoder::LameEncoder()
{
}

LameEncoder::~LameEncoder()
{
}

bool LameEncoder::GetOptions(vector<const BaseOption*>& list)
{
    list.clear();
    return true;
}
