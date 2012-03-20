#ifndef MOUS_IPLAYER_EVENT_LISTENER_H
#define MOUS_IPLAYER_EVENT_LISTENER_H

namespace mous {

class IPlayer;

class IPlayerEventListener
{
public:
    virtual ~IPlayerEventListener() { }

    virtual void SetEventProvider(const IPlayer* player) = 0;
    virtual void UnsetEventProvider() = 0;
};

}

#endif
