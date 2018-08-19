#pragma once

#include <inttypes.h>
#include <string>
#include <vector>
#include <util/AudioMode.h>
#include <util/ErrorCode.h>
#include <util/Option.h>
#include <util/Plugin.h>
#include "Interface.h"
#include "InterfaceWrapper.h"

namespace mous {

class Decoder {
    COPY_INTERFACE_WRAPPER_COMMON(Decoder, DecoderInterface, "MousGetDecoderInterface");

public:
    ErrorCode Open(const std::string& url) {
        return m_interface->open(m_data, url.c_str());
    }

    void Close() {
        return m_interface->close(m_data);
    }

    ErrorCode DecodeUnit(char* data, uint32_t& used, uint32_t& uint_count) {
        return m_interface->decode_unit(m_data, data, &used, &uint_count);
    }

    ErrorCode SetUnitIndex(uint64_t index) {
        return m_interface->set_unit_index(m_data, index);
    }

    uint32_t MaxBytesPerUnit() const {
        return m_interface->get_max_bytes_per_unit(m_data);
    }

    uint64_t UnitIndex() const {
        return m_interface->get_unit_index(m_data);
    }

    uint64_t UnitCount() const {
        return m_interface->get_unit_count(m_data);
    }

    enum AudioMode AudioMode() const {
        return m_interface->get_audio_mode(m_data);
    }

    int32_t Channels() const {
        return m_interface->get_channels(m_data);
    }

    int32_t BitsPerSample() const {
        return m_interface->get_bits_per_sample(m_data);
    }

    int32_t SampleRate() const {
        return m_interface->get_sample_rate(m_data);
    }

    int32_t BitRate() const {
        return m_interface->get_bit_rate(m_data);
    }

    uint64_t Duration() const {
        return m_interface->get_duration(m_data);
    }

    std::vector<std::string> FileSuffix() const {
        std::vector<std::string> suffixes;
        const char** p = m_interface->get_suffixes(m_data);
        if (p) {
            for (; *p; ++p) {
                suffixes.emplace_back(*p);
            }
        }
        return suffixes;
    }
};

}
