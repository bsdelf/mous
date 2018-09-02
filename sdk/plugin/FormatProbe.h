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

class FormatProbe {
    COPY_INTERFACE_WRAPPER_COMMON(FormatProbe, FormatProbeInterface, "MousGetFormatProbeInterface");
    
public:
    std::string Probe(const std::string& path) const {
        const char* format = m_interface->probe(m_data, path.c_str());
        return { format ? format : "" };
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
