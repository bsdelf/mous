#ifndef MOUS_CONVTASK_H
#define MOUS_CONVTASK_H

#include <common/MediaItem.h>
#include <core/IConvTask.h>
#include <plugin/IDecoder.h>
#include <plugin/IEncoder.h>

namespace mous {

class ConvTask: public IConvTask
{
public:
    explicit ConvTask(const MediaItem*, IDecoder*, IEncoder*);
    virtual ~ConvTask();

    virtual bool GetDecoderOptions(std::vector<const BaseOption*>& list) const;
    virtual bool GetEncoderOptions(std::vector<const BaseOption*>& list) const;

    virtual EmErrorCode Run();
    virtual void Cancel();

    virtual int GetProgress() const;

private:
    MediaItem m_Item;
    IDecoder* m_Decoder;
    IEncoder* m_Encoder;
};

}

#endif
