#include "Playlist.h"
#include <algorithm>
using namespace std;
using namespace mous;

Playlist::Playlist():
    m_PlayMode(PlayMode::Normal)
{

}

Playlist::~Playlist()
{

}

void Playlist::SetPlayMode(EmPlayMode mode)
{
    m_PlayMode = mode;
}

EmPlayMode Playlist::GetPlayMode() const
{
    return m_PlayMode;
}

MediaItem* Playlist::GetPreviousItem()
{
    return NULL;
}

MediaItem* Playlist::GetCurrentItem()
{
    return NULL;
}

MediaItem* Playlist::GetNextItem()
{
    return NULL;
}

void Playlist::InsertItem(size_t index, MediaItem* item)
{
    m_ItemQue.insert(m_ItemQue.begin()+index, item);
}

void Playlist::AppendItem(MediaItem* item)
{
    m_ItemQue.push_back(item);
}

void Playlist::RemoveItem(size_t index)
{
    m_ItemQue.erase(m_ItemQue.begin() + index);
}

void Playlist::Clear()
{
    m_ItemQue.clear();
}

MediaItem* Playlist::GetItem(size_t index)
{
    return m_ItemQue[index];
}

size_t Playlist::GetItemCount() const
{
    return m_ItemQue.size();
}

void Playlist::Reverse()
{
    reverse(m_ItemQue.begin(), m_ItemQue.end());
}
