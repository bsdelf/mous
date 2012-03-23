#ifndef MOUS_IEVENT_WATCHER_H
#define MOUS_IEVENT_WATCHER_H

namespace mous {

class IPlayer;

class IEventWatcher
{
public:
    virtual ~IEventWatcher() { }

    virtual void SetPlayer(const IPlayer* player) { }
    virtual void UnsetPlayer() { }
};

}

#endif
