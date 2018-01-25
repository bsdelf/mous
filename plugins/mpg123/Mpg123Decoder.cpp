#include "Mpg123Decoder.h"

#include <unistd.h> // for off_t
#include <string.h>
#include <cassert>

Mpg123Decoder::Mpg123Decoder()
{
    int error = mpg123_init();
    auto decoders = mpg123_supported_decoders();
    assert(decoders && decoders[0]);
    handle_ = mpg123_parnew(nullptr, decoders[0], &error);
    mpg123_param(handle_, MPG123_FLAGS, MPG123_QUIET | MPG123_SKIP_ID3V2, 0);
    max_bytes_per_unit_ = mpg123_safe_buffer();
}

Mpg123Decoder::~Mpg123Decoder()
{
    if (handle_ != nullptr) {
        mpg123_close(handle_);
        mpg123_delete(handle_);
    }
    mpg123_exit();
}

vector<string> Mpg123Decoder::FileSuffix() const
{
    return { "mp3" };
}

ErrorCode Mpg123Decoder::Open(const std::string& url)
{
    if (!handle_) {
        return ErrorCode::DecoderFailedToInit;
    }

    if (mpg123_open(handle_, url.c_str()) != MPG123_OK) {
        return ErrorCode::DecoderFailedToOpen;
    }

    mpg123_scan(handle_);
    mpg123_seek_frame(handle_, 0, SEEK_END);
    unit_count_ = mpg123_tellframe(handle_);
    mpg123_seek_frame(handle_, 0, SEEK_SET);

    if (unit_count_ <= 0) {
        return ErrorCode::DecoderFailedToOpen;
    }

    long sampleRate;
    int channels;
    int encoding;
    mpg123_getformat(handle_, &sampleRate, &channels, &encoding);

    channels_ = channels;
    sample_rate_ = sampleRate;
    switch (encoding) {
        case MPG123_ENC_SIGNED_16:
        case MPG123_ENC_UNSIGNED_16:
        case MPG123_ENC_16:
            bits_per_sample_ = 16;
            break;
        default:
            bits_per_sample_ = 8;
    }

    duration_ = mpg123_tpf(handle_) * 1000.f * unit_count_;
    unit_index_ = 0;

    return ErrorCode::Ok;
}

void Mpg123Decoder::Close()
{
    if (handle_ != nullptr) {
        mpg123_close(handle_);
    }
}

bool Mpg123Decoder::IsFormatVaild() const
{
    return true;
}

ErrorCode Mpg123Decoder::DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    if (unit_index_ >= unit_count_) {
        used = 0;
        return ErrorCode::DecoderOutOfRange;
    }

    mpg123_frameinfo info;
    mpg123_info(handle_, &info);
    bit_rate_ = info.bitrate;

    size_t _len;
    mpg123_replace_buffer(handle_, reinterpret_cast<unsigned char*>(data), max_bytes_per_unit_);
    mpg123_decode_frame(handle_, (off_t*)&unit_index_, nullptr, &_len);
    used = _len;
    unitCount = 1;
    ++unit_index_;

    return ErrorCode::Ok;
}

ErrorCode Mpg123Decoder::SetUnitIndex(uint64_t index)
{
    if (index >= unit_count_) {
        return ErrorCode::DecoderOutOfRange;
    }
    unit_index_ = index;
    mpg123_seek_frame(handle_, unit_index_, SEEK_SET);
    return ErrorCode::Ok;
}

uint32_t Mpg123Decoder::MaxBytesPerUnit() const
{
    return max_bytes_per_unit_;
}

uint64_t Mpg123Decoder::UnitIndex() const
{
    return unit_index_;
}

uint64_t Mpg123Decoder::UnitCount() const
{
    return unit_count_;
}

AudioMode Mpg123Decoder::AudioMode() const
{
    return AudioMode::Stereo;
}

int32_t Mpg123Decoder::Channels() const
{
    return channels_;
}

int32_t Mpg123Decoder::BitsPerSample() const
{
    return bits_per_sample_;
}

int32_t Mpg123Decoder::SampleRate() const
{
    return sample_rate_;
}

int32_t Mpg123Decoder::BitRate() const
{
    return bit_rate_; 
}

uint64_t Mpg123Decoder::Duration() const
{
    return duration_;
}
