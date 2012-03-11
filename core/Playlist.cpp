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

const MediaItem* Playlist::SeqPrevious() const
{
    return NULL;
}

const MediaItem* Playlist::SeqCurrent() const
{
    return NULL;
}

const MediaItem* Playlist::SeqNext() const
{
    return NULL;
}

EmErrorCode Playlist::SeqMoveNext(int step)
{
    return ErrorCode::Ok;
}

void Playlist::ResetSeq()
{
}

void Playlist::AssignItems(std::deque<MediaItem*>& items)
{
    m_ItemQue.assign(items.begin(), items.end());
}

void Playlist::InsertItem(size_t index, MediaItem* item)
{
    m_ItemQue.insert(m_ItemQue.begin()+index, item);
}

void Playlist::InsertItems(size_t index, deque<MediaItem*>& items)
{
    m_ItemQue.insert(m_ItemQue.begin()+index, items.begin(), items.end());
}

void Playlist::AppendItem(MediaItem* item)
{
    m_ItemQue.push_back(item);
}

void Playlist::AppendItems(deque<MediaItem*>& items)
{
    m_ItemQue.insert(m_ItemQue.end(), items.begin(), items.end());
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

bool Playlist::Empty() const
{
    return m_ItemQue.empty();
}

void Playlist::Reverse()
{
    reverse(m_ItemQue.begin(), m_ItemQue.end());
}
