#pragma once

#include <vector>
#include <deque>
#include <string>
#include <util/Option.h>
#include <util/MediaItem.h>
#include "Interface.h"
#include "InterfaceWrapper.h"

namespace mous {

struct MediaItem;

class SheetParser
{
    COPY_INTERFACE_WRAPPER_COMMON(SheetParser, SheetParserInterface, "MousGetSheetParserInterface");

public:
    void DumpFile(const std::string& path, std::deque<MediaItem>& list) const {
        return m_interface->dump_file(m_data, path.c_str(), &list);
    }

    void DumpStream(const std::string& stream, std::deque<MediaItem>& list) const {
        return m_interface->dump_stream(m_data, stream.c_str(), &list);
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
