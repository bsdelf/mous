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

const MediaItem* Playlist::GetPreviousItem() const
{
    return NULL;
}

const MediaItem* Playlist::GetCurrentItem() const
{
    return NULL;
}

const MediaItem* Playlist::GetNextItem() const
{
    return NULL;
}

bool Playlist::MoveNext(bool forward)
{
    return true;
}

void Playlist::ResetSeq()
{
}

void Playlist::InsertItem(size_t index, MediaItem* item)
{
    m_ItemQue.insert(m_ItemQue.begin()+index, item);
}

void Playlist::InsertItems(size_t index, deque<MediaItem*>& items)
{
}

void Playlist::AppendItem(MediaItem* item)
{
    m_ItemQue.push_back(item);
}

void Playlist::AppendItems(deque<MediaItem*>& items)
{
}

void Playlist::RemoveItem(size_t index)
{
    m_ItemQue.erase(m_ItemQue.begin() + index);
}

void Playlist::Clear()
{
    m_ItemQue.clear();
}

const MediaItem* Playlist::GetItem(size_t index) const
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
