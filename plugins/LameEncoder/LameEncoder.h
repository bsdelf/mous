#ifndef LAMEENCODER_H
#define LAMEENCODER_H

#include <plugin/IEncoder.h>
using namespace mous;

class LameEncoder: public IEncoder
{
public:
    LameEncoder();
    ~LameEncoder();

    bool GetOptions(std::vector<const BaseOption*>& list) const;

private:
};

#endif
