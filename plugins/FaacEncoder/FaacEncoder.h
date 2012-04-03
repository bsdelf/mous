#ifndef FAACENCODER_H
#define FAACENCODER_H

#include <plugin/IEncoder.h>
#include <faac.h>
using namespace std;
using namespace mous;

class FaacEncoder: public IEncoder
{
public:
    FaacEncoder();
    virtual ~FaacEncoder();

    virtual const char* GetFileSuffix() const;

    virtual EmErrorCode OpenOutput(const std::string& path);
    virtual void CloseOutput();

    virtual EmErrorCode Encode(char* buf, uint32_t len);
    virtual EmErrorCode FlushRest();

    virtual void SetChannels(int32_t channels);
    virtual void SetSampleRate(int32_t sampleRate);
    virtual void SetBitsPerSample(int32_t bitsPerSample);

    virtual bool GetOptions(std::vector<const BaseOption*>& list) const;

private:
    RangedIntOption m_Quality;
    EnumedIntOption m_BitRate;
    BooleanOption m_ReplayGain;
};

#endif
