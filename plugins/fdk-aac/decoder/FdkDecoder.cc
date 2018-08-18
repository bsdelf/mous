#include "FdkDecoder.h"
#include <stdio.h>
#include <iostream>
using namespace std;

FdkDecoder::FdkDecoder()
{

}

FdkDecoder::~FdkDecoder()
{
}

vector<string> FdkDecoder::FileSuffix() const
{
    return { "m4a", "mp4" };
}

ErrorCode FdkDecoder::Open(const string& url)
{
    // check for mp4 file
    m_ismp4 = false;

    FILE* file = fopen(url.c_str(), "rb");
    if (file == nullptr) {
        return ErrorCode::DecoderFailedToOpen;
    }

    unsigned char header[8];
    size_t nch = fread(header, 1, 8, file);
    fclose(file);
    if (nch != 8) {
        return ErrorCode::DecoderFailedToOpen;
    }

    if (header[4] == 'f' && header[5] == 't' && 
            header[6] == 'y' && header[7] == 'p') {
        m_ismp4 = true;
    }

    return m_ismp4 ? OpenMP4(url) : OpenAAC(url);
}

ErrorCode FdkDecoder::OpenMP4(const string& url)
{
    // mp4v2
    m_mp4 = MP4Read(url.c_str());
    if (m_mp4 == nullptr) {
        return ErrorCode::DecoderFailedToOpen;
    }

    {
        // get first audio track
        m_trackid = 0;
        int ntrack = MP4GetNumberOfTracks(m_mp4);
        for (int itrack = 1; itrack <= ntrack; ++itrack) {
            if (MP4_IS_AUDIO_TRACK_TYPE(MP4GetTrackType(m_mp4, itrack))) {
                m_trackid = itrack;
                cout << m_trackid << endl;
                break;
            }
        }
        if (m_trackid == 0) {
            Close();
            return ErrorCode::DecoderFailedToOpen;
        }
    }

    m_bitrate = MP4GetTrackBitRate(m_mp4, m_trackid);
    m_duration = MP4ConvertFromTrackDuration(
        m_mp4, m_trackid, MP4GetTrackDuration(m_mp4, m_trackid), MP4_MSECS_TIME_SCALE);

    m_channels = MP4GetTrackAudioChannels(m_mp4, m_trackid);
    m_timescale = MP4GetTrackTimeScale(m_mp4, m_trackid);
    m_bits = 16;

    m_nsamples = MP4GetTrackNumberOfSamples(m_mp4, m_trackid);
    m_sampleid = 1; // player will call SetUnitIndex() anyway
    m_samplebuff.resize(MP4GetTrackMaxSampleSize(m_mp4, m_trackid));
    m_samplebuff.shrink_to_fit();

    //cout << "info: " << m_bitrate << ", "<< m_channels << ", "<< m_nsamples << ", " << m_timescale << endl;

    // fdk
    m_decoder = aacDecoder_Open(TT_MP4_RAW, 1);
    if (m_decoder == nullptr) {
        Close();
        return ErrorCode::DecoderFailedToOpen;
    }

    unsigned char conf[128];
    UINT confBytes = sizeof(conf);
    MP4GetTrackESConfiguration(m_mp4, m_trackid, (uint8_t **) &conf, (uint32_t *) &confBytes);
    aacDecoder_ConfigRaw(m_decoder, (unsigned char **)&conf, (uint32_t *) &confBytes);

    // see AACDEC_PARAM
    //aacDecoder_SetParam(m_decoder, AAC_PCM_OUTPUT_INTERLEAVED, 1);
    //aacDecoder_SetParam(m_decoder, AAC_PCM_OUTPUT_CHANNELS, m_channels);

    return ErrorCode::Ok;
}

ErrorCode FdkDecoder::OpenAAC(const string& url)
{
    return ErrorCode::DecoderFailedToOpen;
}

void FdkDecoder::Close()
{
    if (m_mp4 != nullptr) {
        MP4Close(m_mp4);
        m_mp4 = nullptr;
    }
    if (m_decoder != nullptr) {
        aacDecoder_Close(m_decoder);
        m_decoder = nullptr;
    }
}

ErrorCode FdkDecoder::DecodeUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    return m_ismp4 ? 
        DecodeMp4Unit(data, used, unitCount) : DecodeAacUnit(data, used, unitCount);
}

ErrorCode FdkDecoder::DecodeMp4Unit(char* data, uint32_t& used, uint32_t& unitCount)
{
    // read sample
    uint8_t* pbytes = m_samplebuff.data();
    uint32_t nbytes = m_samplebuff.size();
    bool ok = MP4ReadSample(m_mp4, m_trackid, m_sampleid, &pbytes, &nbytes);
    if (!ok) {
        cout << "mp4 bad sample: " << m_sampleid << endl;
        return ErrorCode::DecoderFailedToRead;
    }
    //cout << nbytes << " - " << m_samplebuff.size() << endl;

    // decode sample
    UINT valid = nbytes;
    AAC_DECODER_ERROR err = aacDecoder_Fill(m_decoder, &pbytes, &nbytes, &valid);
    if (err != AAC_DEC_OK) {
        cout << "fdk bad fill" << endl;
        return ErrorCode::DecoderFailedToRead;
    }

    err = aacDecoder_DecodeFrame(m_decoder, reinterpret_cast<INT_PCM*>(data), MaxBytesPerUnit(), 0);
    if (err == AAC_DEC_NOT_ENOUGH_BITS) {
        cout << "fdk short frame" << endl;
        return ErrorCode::DecoderFailedToRead;
    }
    if (err != AAC_DEC_OK) {
        cout << "fdk bad frame" << endl;
        return ErrorCode::DecoderFailedToRead;
    }

    auto framesize = aacDecoder_GetStreamInfo(m_decoder)->frameSize;
    used = m_channels * framesize * (m_bits / 8);
    unitCount = 1;

    // move forward
    m_sampleid += 1;

    return ErrorCode::Ok;
}

ErrorCode FdkDecoder::DecodeAacUnit(char* data, uint32_t& used, uint32_t& unitCount)
{
    return ErrorCode::Ok;
}

ErrorCode FdkDecoder::SetUnitIndex(uint64_t index)
{
    m_sampleid = index + 1;
    return ErrorCode::Ok;
}

uint32_t FdkDecoder::MaxBytesPerUnit() const
{
    /* Frame size of decoded PCM signal
     * 1024 or 960 for AAC-LC
     * 2048 or 1920 for HE-AAC (v2)
     * 512 or 480 for AAC-LD and AAC-ELD
     */
    return 2048 * m_channels;
}

uint64_t FdkDecoder::UnitIndex() const
{
    return m_sampleid - 1;
}

uint64_t FdkDecoder::UnitCount() const
{
    return m_nsamples;
}

AudioMode FdkDecoder::AudioMode() const
{
    return AudioMode::Stereo;
}

int32_t FdkDecoder::Channels() const
{
    return m_channels;
}

int32_t FdkDecoder::BitsPerSample() const
{
    return m_bits;
}

int32_t FdkDecoder::SampleRate() const
{
    return m_timescale;
}

int32_t FdkDecoder::BitRate() const
{
    return m_bitrate / 1000;
}

uint64_t FdkDecoder::Duration() const
{
    return m_duration;
}
