#include "PlayerStatistics.h"
#include <iostream>
using namespace std;

PlayerStatistics::PlayerStatistics():
    m_StartTimes(0)
{
    cout << "PlayerStatistics()" << endl;
}

PlayerStatistics::~PlayerStatistics()
{
    cout << "~PlayerStatistics()" << endl;
    cout << "PlayerStatistics::m_StartTimes " << m_StartTimes << endl;
}

void PlayerStatistics::SetPlayer(const IPlayer* player)
{
    player->SigStartPlay()->Connect(&PlayerStatistics::SlotStartPlay, this);
    m_Player = player;
}

void PlayerStatistics::UnsetPlayer()
{
    m_Player->SigStartPlay()->DisconnectReceiver(this);
    m_Player = NULL;
}

void PlayerStatistics::SlotStartPlay()
{
    ++m_StartTimes;
    cout << "SlotStartPlay()" << m_StartTimes << endl;
}

void PlayerStatistics::SlotStopPlaying()
{
}
