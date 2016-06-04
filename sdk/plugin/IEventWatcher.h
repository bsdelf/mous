#pragma once

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
