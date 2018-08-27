#include <algorithm>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <alsa/asoundlib.h>

#include <string>
using namespace std;

#include <plugin/OutputProto.h>
using namespace mous;

template <typename T>
struct Tuple {
    T val;
    T max;
    T min;
};

namespace {
    struct Self {
        string device_name { "default" };

        snd_pcm_t* snd_pcm = nullptr;

        int dir = 0;
        int resample = 1;
        snd_pcm_access_t access;
        snd_pcm_format_t format;

        int frame_length = 0;
        int bits_per_sample = 0;
        Tuple<unsigned int> channels;
        Tuple<unsigned int> sample_rate;
        Tuple<unsigned int> buffer_time;
        Tuple<unsigned int> period_time;
        Tuple<snd_pcm_uframes_t> buffer_size;
        Tuple<snd_pcm_uframes_t> period_size;
    };
}

static bool SetupHwParams(void*);
static void SetupSwParams(void*);

static void* Create() {
    return new Self();
}

static void Destroy(void* ptr) {
    Close(ptr);
    delete SELF;
}

static ErrorCode Open(void* ptr) {
    int ret = snd_pcm_open(&SELF->snd_pcm, SELF->device_name.c_str(), SND_PCM_STREAM_PLAYBACK, 0);
    if (ret != 0) {
        return ErrorCode::OutputFailedToOpen;
    }
    return ErrorCode::Ok;
}

static void Close(void* ptr) {
    if (SELF->snd_pcm) {
        snd_pcm_drain(SELF->snd_pcm);
        snd_pcm_close(SELF->snd_pcm);
        SELF->snd_pcm = nullptr;
    }
}

static ErrorCode Setup(void* ptr, int32_t* channels, int32_t* sample_rate, int32_t* bits_per_sample) {
    SELF->channels.val = *channels;
    SELF->sample_rate.val = *sample_rate;
    SELF->bits_per_sample = *bits_per_sample;

    bool ok = SetupHwParams(ptr);
    if (ok) {
        SetupSwParams(ptr);
    }

    if ((unsigned int)*channels != SELF->channels.val) {
        *channels = SELF->channels.val;
        ok = false;
    }
    if ((unsigned int)*sample_rate != SELF->sample_rate.val) {
        *sample_rate = SELF->sample_rate.val;
        //NOTE: just is a workaround :-(
        ok = *sample_rate > 1 ? true : false;
    }
    if (*bits_per_sample != SELF->bits_per_sample) {
        *bits_per_sample = SELF->bits_per_sample;
        ok = false;
    }

    return ok ? ErrorCode::Ok : ErrorCode::OutputFailedToSetup;
}

static ErrorCode Write(void* ptr, const char* data, uint32_t length) {
    int off = 0;
    int left_frames = (length + SELF->frame_length - 1) / SELF->frame_length;

    while (left_frames > 0) {
        int written = snd_pcm_writei(SELF->snd_pcm, data + off, left_frames);
        if (written > 0) {
            left_frames -= written;
            off += snd_pcm_frames_to_bytes(SELF->snd_pcm, written);
        } else if (written == -EPIPE) {
            if (snd_pcm_prepare(SELF->snd_pcm) < 0)
                printf("FATAL: snd_pcm_prepare() failed!\n");
        } else if (written == -ESTRPIPE) {
            while ((written = snd_pcm_resume(SELF->snd_pcm)) == -EAGAIN) {
                usleep(100);
            }
            if (written < 0) {
                if ((written = snd_pcm_prepare(SELF->snd_pcm)) < 0) {
                    return ErrorCode::OutputFailedToWrite;
                }
            }
        } else if (written <= 0) {
            printf("writei error / short write\n");
        }
    }

    return ErrorCode::Ok;
}

static int GetVolume(void* ptr) {
    return 0;
}

static void SetVolume(void* ptr, int level) {
}

static bool SetupHwParams(void* ptr) {
    snd_pcm_hw_params_t* params;

    /* allocate a hardware parameters object */
    snd_pcm_hw_params_malloc(&params);

    /* choose all parameters */
    snd_pcm_hw_params_any(SELF->snd_pcm, params);

    /* enable hardware resampling */
    snd_pcm_hw_params_set_rate_resample(SELF->snd_pcm, params, SELF->resample);

    /* set the interleaved read/write format */
    snd_pcm_hw_params_set_access(SELF->snd_pcm, params, SND_PCM_ACCESS_RW_INTERLEAVED);

    /* signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(SELF->snd_pcm, params, SND_PCM_FORMAT_S16_LE);

    /* set the count of channels */
    snd_pcm_hw_params_get_channels_max(params, &SELF->channels.max);
    snd_pcm_hw_params_get_channels_min(params, &SELF->channels.min);
    if (SELF->channels.val < SELF->channels.min || SELF->channels.val > SELF->channels.max) 
        SELF->channels.val = SELF->channels.min;
    snd_pcm_hw_params_set_channels(SELF->snd_pcm, params, SELF->channels.val);

    /* set the stream rate */
    snd_pcm_hw_params_get_rate_max(params, &SELF->sample_rate.max, &SELF->dir);
    snd_pcm_hw_params_get_rate_min(params, &SELF->sample_rate.min, &SELF->dir);
    if (SELF->sample_rate.val < SELF->sample_rate.min || SELF->sample_rate.val > SELF->sample_rate.max)
        SELF->sample_rate.val = SELF->sample_rate.min;
    printf("sample rate max:%d, min:%d, val:%d\n", 
            SELF->sample_rate.max, SELF->sample_rate.min, SELF->sample_rate.val);
    snd_pcm_hw_params_set_rate(SELF->snd_pcm, params, SELF->sample_rate.val, SELF->dir);

    /* we can set period and buffer by size or time */
    /* by default we use "size" (count of frames)   */

    /* set how many frames in a buffer (a buffer cantains several periods) */
    snd_pcm_hw_params_get_buffer_size_max(params, &SELF->buffer_size.max);
    snd_pcm_hw_params_get_buffer_size_min(params, &SELF->buffer_size.min);

    /* detect period time and buffer time range  */
    snd_pcm_hw_params_get_buffer_time_max(params, &SELF->buffer_time.max, &SELF->dir);
    snd_pcm_hw_params_get_buffer_time_min(params, &SELF->buffer_time.min, &SELF->dir);
    SELF->buffer_time.val = std::max(SELF->buffer_time.max / 2, SELF->buffer_time.min); 
    //SELF->buffer_time.val = SELF->buffer_time.min + (SELF->buffer_time.max - SELF->buffer_time.min) /2; 
    printf("buffer time max:%d, min:%d, val:%d\n", 
            SELF->buffer_time.max, SELF->buffer_time.min, SELF->buffer_time.val);
    //(SELF->buffer_time.max > 120000) ? 120000 : SELF->buffer_time.max;//120000
    snd_pcm_hw_params_set_buffer_time_near(SELF->snd_pcm, params, &SELF->buffer_time.val, &SELF->dir);

    snd_pcm_hw_params_get_period_time_max(params, &SELF->period_time.max, &SELF->dir);
    snd_pcm_hw_params_get_period_time_min(params, &SELF->period_time.min, &SELF->dir);
    SELF->period_time.val = SELF->buffer_time.val / 4;
    //SELF->period_time.val = SELF->period_time.min + (SELF->period_time.max - SELF->period_time.min) / 2;
    printf("period time max:%d, min:%d, val:%d\n", 
            SELF->period_time.max, SELF->period_time.min, SELF->period_time.val);
    snd_pcm_hw_params_set_period_time_near(SELF->snd_pcm, params, &SELF->period_time.val, &SELF->dir);

    int ret = snd_pcm_hw_params(SELF->snd_pcm, params);

    if (ret != 0) {
        snd_pcm_hw_params_free(params);
        return false;
    }

    // get period size again
    snd_pcm_hw_params_get_period_size(params, &SELF->period_size.val, &SELF->dir);
    snd_pcm_hw_params_get_buffer_size(params, &SELF->buffer_size.val);

    SELF->bits_per_sample = snd_pcm_format_physical_width(SND_PCM_FORMAT_S16_LE);
    SELF->frame_length = (SELF->bits_per_sample/8 * SELF->channels.val);
    //mPeriodBufferLength = SELF->period_size.val * SELF->frame_length ;

    snd_pcm_hw_params_free(params);

    return true;
}

static void SetupSwParams(void* ptr) {
    snd_pcm_sw_params_t* params;

    /* allocate a software parameters object */
    snd_pcm_sw_params_malloc(&params);

    /* get the current swparams */
    snd_pcm_sw_params_current(SELF->snd_pcm, params);

    /* round up to closest transfer boundary */
    snd_pcm_sw_params_set_start_threshold(SELF->snd_pcm, params, 1);

    /* require a minimum of one full transfer in the buffer */
    snd_pcm_sw_params_set_avail_min(SELF->snd_pcm, params, 1);

    snd_pcm_sw_params(SELF->snd_pcm, params);

    /* free */
    snd_pcm_sw_params_free(params);
}

static const BaseOption** GetOptions(void* ptr) {
    (void) ptr;
    return nullptr;
}