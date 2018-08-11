#include "AoOutput.h"
#include <iostream>
#include <string.h>

AoOutput::AoOutput():
    m_Driver(-1),
    m_Device(nullptr)
{
    ao_initialize();

    m_Driver = ao_default_driver_id();

    int channels = 2;
    int rate = 44100;
    int bits = 16;
    Setup(channels, rate, bits);

    {
        int count;
        ao_info** info = ao_driver_info_list(&count);
        cout << "  driver count:" << count << endl;
        for (int i = 0; i < count; ++i) {
            cout << " " << i << " " << info[i]->name << endl;
            for (int j = 0; j < info[i]->option_count; ++j) {
                cout << "   " << j << " " <<  info[i]->options[j] << endl;
            }
        }
    }
}

AoOutput::~AoOutput()
{
    Close();

    ao_shutdown();
}

EmErrorCode AoOutput::Open()
{
    Close();

    m_Device = ao_open_live(m_Driver, &m_Format, nullptr);
    return m_Device != nullptr ? ErrorCode::Ok : ErrorCode::OutputFailedToOpen;
}

void AoOutput::Close()
{
    if (m_Device != nullptr) {
        ao_close(m_Device);
        m_Device = nullptr;
    }
}

EmErrorCode AoOutput::Setup(int32_t& channels, int32_t& sampleRate, int32_t& bitsPerSample)
{
    memset(&m_Format, 0, sizeof(m_Format));
    m_Format.channels = channels;
    m_Format.bits = bitsPerSample;
    m_Format.rate = sampleRate;
    m_Format.byte_format = AO_FMT_LITTLE;
    return Open() == ErrorCode::Ok ? ErrorCode::Ok : ErrorCode::OutputFailedToSetup;
}

EmErrorCode AoOutput::Write(const char* buf, uint32_t len)
{
    int ret = ao_play(m_Device, const_cast<char*>(buf), (uint_32)len);
    return ret == 0 ? ErrorCode::Ok : ErrorCode::OutputFailedToWrite;
}

int AoOutput::VolumeLevel() const
{
    return 0;
}

void AoOutput::SetVolumeLevel(int level)
{
}
