#pragma once

#include <inttypes.h>
#include <vector>
#include <memory>
#include <string>
#include <util/ErrorCode.h>
#include <util/Option.h>
#include <util/Plugin.h>
#include "Interface.h"
#include "InterfaceWrapper.h"

namespace mous {

class Output {
    COPY_INTERFACE_WRAPPER_COMMON(Output, OutputInterface, "MousGetOutputInterface");
    
public:
    ErrorCode Open() {
        return m_interface->open(m_data);
    }

    void Close() {
        return m_interface->close(m_data);
    }

    ErrorCode Setup(int32_t& channels, int32_t& sample_rate, int32_t& bits_per_sample) {
        return m_interface->setup(m_data, &channels, &sample_rate, &bits_per_sample);
    }

    ErrorCode Write(const char* data, uint32_t length) {
        return m_interface->write(m_data, data, length);
    }

    int GetVolume() const {
        return m_interface->get_volume(m_data);
    }

    void SetVolume(int level) {
        return m_interface->set_volume(m_data, level);
    }
};

}
