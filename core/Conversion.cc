#include <core/Conversion.h>
#include "ConversionImpl.h"

namespace mous {

Conversion::Conversion(const MediaItem& item,
                       const std::shared_ptr<Plugin>& decoderPlugin,
                       const std::shared_ptr<Plugin>& encoderPlugin)
    : impl(std::make_unique<Impl>(item, decoderPlugin, encoderPlugin)) {
}

Conversion::~Conversion() {
}

std::vector<const BaseOption*> Conversion::DecoderOptions() const {
  return impl->DecoderOptions();
}

std::vector<const BaseOption*> Conversion::EncoderOptions() const {
  return impl->EncoderOptions();
}

std::string Conversion::EncoderFileSuffix() const {
  return impl->EncoderFileSuffix();
}

void Conversion::Run(const std::string& output) {
  return impl->Run(output);
}

void Conversion::Cancel() {
  return impl->Cancel();
}

double Conversion::Progress() const {
  return impl->Progress();
}

bool Conversion::IsFinished() const {
  return impl->IsFinished();
}

}  // namespace mous
