#pragma once

#include <string>
#include <vector>
#include <inttypes.h>
#include <util/AudioMode.h>
#include <util/ErrorCode.h>
#include <util/MediaTag.h>
#include <util/Option.h>
#include "Interface.h"
#include "InterfaceWrapper.h"

namespace mous {

class Encoder {
    COPY_INTERFACE_WRAPPER_COMMON(Encoder, EncoderInterface, "MousGetEncoderInterface");

public:
    // these methods will be called before OpenOutput()
    void SetChannels(int32_t channels) {
        return m_interface->set_channels(m_data, channels);
    }

    void SetSampleRate(int32_t sample_rate) {
        return m_interface->set_sample_rate(m_data, sample_rate);
    }

    void SetBitsPerSample(int32_t bits_per_sample) {
        return m_interface->set_bits_per_sample(m_data, bits_per_sample);
    }

    // called before OpenOutput()
    // you can write tag after open but before close
    void SetMediaTag(const MediaTag* tag) {
        return m_interface->set_media_tag(m_data, tag);
    }

    ErrorCode OpenOutput(const std::string& path) {
        return m_interface->open_output(m_data, path.c_str());
    }

    void CloseOutput() {
        return m_interface->close_output(m_data);
    }

    ErrorCode Encode(const char* data, uint32_t length) {
        return m_interface->encode(m_data, data, length);
    }

    ErrorCode Flush() {
        return m_interface->flush(m_data);
    }

    const char* FileSuffix() const {
        return m_interface->get_suffix(m_data);
    }
};

}
