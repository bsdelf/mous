#pragma once

#include <thread>
using namespace std;

#include <util/MediaItem.h>
#include <core/IConvTask.h>
#include <plugin/IDecoder.h>
#include <plugin/IEncoder.h>
using namespace mous;

namespace mous {

class ConvTask: public IConvTask
{
public:
    explicit ConvTask(const MediaItem&, const Plugin*, const Plugin*);
    virtual ~ConvTask();

    virtual std::vector<const BaseOption*> DecoderOptions() const;
    virtual std::vector<const BaseOption*> EncoderOptions() const;
    virtual const char* EncoderFileSuffix() const;

    virtual void Run(const string& output);
    virtual void Cancel();

    virtual double Progress() const;
    virtual bool IsFinished() const;

private:
    MediaItem m_Item;
    const Plugin* m_DecAgent;
    const Plugin* m_EncAgent;
    std::thread m_WorkThread;

    IDecoder* m_Decoder;
    IEncoder* m_Encoder;

    double m_Progress = -1;
    bool m_Finished = true;
    bool m_Canceled = false;
};

}
