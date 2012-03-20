#ifndef PLAYERSTATISTICS_H
#define PLAYERSTATISTICS_H

#include <plugin/IPlayerEventListener.h>
#include <core/IPlayer.h>
#include <scx/AsyncSignal.hpp>
using namespace mous;

class PlayerStatistics: public IPlayerEventListener
{
public:
    PlayerStatistics();
    ~PlayerStatistics();

    virtual void SetEventProvider(const IPlayer* player);
    virtual void UnsetEventProvider();

public:
    void SlotStartPlay();
    void SlotStopPlaying();

private:
    const IPlayer* m_Player;

    int m_StartTimes;
};

#endif
