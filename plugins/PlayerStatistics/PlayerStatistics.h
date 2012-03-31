#ifndef PLAYERSTATISTICS_H
#define PLAYERSTATISTICS_H

#include <plugin/IEventWatcher.h>
#include <core/IPlayer.h>
#include <scx/Signal.hpp>
using namespace mous;

class PlayerStatistics: public IEventWatcher
{
public:
    PlayerStatistics();
    ~PlayerStatistics();

    virtual void SetPlayer(const IPlayer* player);
    virtual void UnsetPlayer();

public:
    void SlotStartPlay();
    void SlotStopPlaying();

private:
    const IPlayer* m_Player;

    int m_StartTimes;
};

#endif
