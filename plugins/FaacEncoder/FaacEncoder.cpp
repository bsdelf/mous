#include "FaacEncoder.h"

FaacEncoder::FaacEncoder()
{
}

FaacEncoder::~FaacEncoder()
{
}

const char* FaacEncoder::GetFileSuffix() const
{
    return "m4a";
}

EmErrorCode FaacEncoder::OpenOutput(const std::string& path)
{
    return ErrorCode::Ok;
}

void FaacEncoder::CloseOutput()
{
}

EmErrorCode FaacEncoder::Encode(char* buf, uint32_t len)
{
    return ErrorCode::Ok;
}

EmErrorCode FaacEncoder::FlushRest()
{
    return ErrorCode::Ok;
}

void FaacEncoder::SetChannels(int32_t channels)
{
}

void FaacEncoder::SetSampleRate(int32_t sampleRate)
{
}

void FaacEncoder::SetBitsPerSample(int32_t bitsPerSample)
{
}

bool FaacEncoder::GetOptions(std::vector<const BaseOption*>& list) const
{
    return true;
}
