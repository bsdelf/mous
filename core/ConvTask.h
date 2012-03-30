#ifndef MOUS_CONVTASK_H
#define MOUS_CONVTASK_H

#include <util/MediaItem.h>
#include <core/IConvTask.h>
#include <plugin/IDecoder.h>
#include <plugin/IEncoder.h>
#include <scx/Thread.hpp>
using namespace std;
using namespace scx;
using namespace mous;

namespace mous {

class ConvTask: public IConvTask
{
public:
    explicit ConvTask(const MediaItem*, const IPluginAgent*, const IPluginAgent*);
    virtual ~ConvTask();

    virtual bool GetDecoderOptions(std::vector<const BaseOption*>& list) const;
    virtual bool GetEncoderOptions(std::vector<const BaseOption*>& list) const;

    virtual void Run(const string& output);
    virtual void Cancel();

    virtual double GetProgress() const;
    virtual bool IsFinished() const;

private:
    void DoConvert(const string& output);

private:
    MediaItem m_Item;
    const IPluginAgent* m_DecAgent;
    const IPluginAgent* m_EncAgent;
    Thread m_WorkThread;

    IDecoder* m_Decoder;
    IEncoder* m_Encoder;

    double m_Progress;
    bool m_Finished;
    bool m_Canceled;
};

}

#endif
