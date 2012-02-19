#include "PlayList.h"
#include <algorithm>
using namespace std;
using namespace mous;

PlayList::PlayList():
    m_PlayMode(PlayMode::Normal)
{

}

PlayList::~PlayList()
{

}

void PlayList::SetPlayMode(EmPlayMode mode)
{
    m_PlayMode = mode;
}

EmPlayMode PlayList::GetPlayMode() const
{
    return m_PlayMode;
}

MediaItem* PlayList::GetPreviousItem()
{
    return NULL;
}

MediaItem* PlayList::GetCurrentItem()
{
    return NULL;
}

MediaItem* PlayList::GetNextItem()
{
    return NULL;
}

void PlayList::InsertItem(size_t index, MediaItem* item)
{
    m_ItemQue.insert(m_ItemQue.begin()+index, item);
}

void PlayList::AppendItem(MediaItem* item)
{
    m_ItemQue.push_back(item);
}

void PlayList::RemoveItem(size_t index)
{
    m_ItemQue.erase(m_ItemQue.begin() + index);
}

void PlayList::Clear()
{
    m_ItemQue.clear();
}

MediaItem* PlayList::GetItem(size_t index)
{
    return m_ItemQue[index];
}

size_t PlayList::GetItemCount() const
{
    return m_ItemQue.size();
}

void PlayList::Reverse()
{
    reverse(m_ItemQue.begin(), m_ItemQue.end());
}
