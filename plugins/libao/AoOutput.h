#pragma once

#include <plugin/IOutput.h>
#include <string>
#include <ao/ao.h>
using namespace std;
using namespace mous;

class AoOutput: public IOutput
{
public:
    AoOutput();
    virtual ~AoOutput();

    virtual ErrorCode Open();
    virtual void Close();

    virtual ErrorCode Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample);
    virtual ErrorCode Write(const char* buf, uint32_t len);

    virtual int VolumeLevel() const;
    virtual void SetVolumeLevel(int level);

private:
    int m_Driver;
    ao_device* m_Device;
    ao_sample_format m_Format;
};

