#include "Playlist.h"
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <iostream>
using namespace std;
using namespace mous;

IPlaylist* IPlaylist::Create()
{
    return new Playlist;
}

void IPlaylist::Free(IPlaylist* ptr)
{
    if (ptr != NULL)
        delete ptr;
}

Playlist::Playlist():
    m_PlayMode(PlayMode::Normal),
    m_SeqNormalIndex(-1),
    m_SeqRepeatIndex(-1),
    m_SeqShuffleIndex(-1)
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

const void* Playlist::SeqCurrent(int off) const
{
    void* item = NULL;
    m_MutexForQue.Lock();

    if (!m_ItemQue.empty()) {
        int idx = -1;
        switch (m_PlayMode) {
            case PlayMode::Normal:
                idx = m_SeqNormalIndex + off;
                break;

            case PlayMode::Repeat:
            case PlayMode::RepeatOne:
                idx = (m_SeqRepeatIndex + off) % m_ItemQue.size();
                break;

            case PlayMode::Shuffle:
                idx = m_SeqShuffleIndex + off;
                break;

            case PlayMode::ShuffleRepeat:
                idx = (m_SeqShuffleIndex + off) % m_ItemQue.size();
                break;
        }
        if (idx >= 0 && idx < m_ItemQue.size())
            item = m_ItemQue[idx];
    }

    m_MutexForQue.Unlock();
    return item;
}

EmErrorCode Playlist::SeqJumpTo(int index) const
{
    EmErrorCode ret = ErrorCode::Ok;
    m_MutexForQue.Lock();
    if (m_ItemQue.empty())
        ret = ErrorCode::PlaylistEmpty;
    else if (index < 0)
        ret = ErrorCode::PlaylistHitBegin;
    else if (index >= m_ItemQue.size())
        ret = ErrorCode::PlaylistHitEnd;
    else
        switch (m_PlayMode) {
            case PlayMode::Normal:
                m_SeqNormalIndex = index;
                break;

            case PlayMode::Repeat:
            case PlayMode::RepeatOne:
                m_SeqRepeatIndex = index;
                break;

            case PlayMode::Shuffle:
            case PlayMode::ShuffleRepeat:
                m_SeqShuffleIndex = index;
                break;
        }
    m_MutexForQue.Unlock();
    return ret;
}

EmErrorCode Playlist::SeqMoveNext(int step) const
{
    EmErrorCode ret = ErrorCode::Ok;
    m_MutexForQue.Lock();
    int* idx = NULL;
    switch (m_PlayMode) {
        case PlayMode::Normal:
            m_SeqNormalIndex += step;
            idx = &m_SeqNormalIndex;
            break;

        case PlayMode::Repeat:
            m_SeqRepeatIndex = (m_SeqRepeatIndex + step) % m_ItemQue.size();
            idx = &m_SeqRepeatIndex;
            break;

        case PlayMode::Shuffle:
            m_SeqShuffleIndex += step;
            idx = &m_SeqShuffleIndex;
            break;

        case PlayMode::ShuffleRepeat:
            m_SeqShuffleIndex = (m_SeqShuffleIndex + step) % m_ItemQue.size();
            idx = &m_SeqShuffleIndex;
            break;

        case PlayMode::RepeatOne:
            idx = &m_SeqRepeatIndex;
            break;
    }
    assert(idx != NULL && *idx >= 0 && *idx < m_ItemQue.size());
    m_MutexForQue.Unlock();
    return ret;
}

void Playlist::AssignItems(std::deque<void*>& items)
{
    m_MutexForQue.Lock();
    m_ItemQue.assign(items.begin(), items.end());
    AdjustSeqIndexes();
    AdjustShuffleRange(true);
    m_MutexForQue.Unlock();
}

void Playlist::InsertItem(int index, void* item)
{
    m_MutexForQue.Lock();
    m_ItemQue.insert(m_ItemQue.begin()+index, item);
    AdjustSeqIndexes();
    AdjustShuffleRange();
    m_MutexForQue.Unlock();
}

void Playlist::InsertItem(int index, deque<void*>& items)
{
    m_MutexForQue.Lock();
    m_ItemQue.insert(m_ItemQue.begin()+index, items.begin(), items.end());
    AdjustSeqIndexes();
    AdjustShuffleRange();
    m_MutexForQue.Unlock();
}

void Playlist::AppendItem(void* item)
{
    m_MutexForQue.Lock();
    m_ItemQue.push_back(item);
    AdjustSeqIndexes();
    AdjustShuffleRange();
    m_MutexForQue.Unlock();
}

void Playlist::AppendItem(deque<void*>& items)
{
    m_MutexForQue.Lock();
    m_ItemQue.insert(m_ItemQue.end(), items.begin(), items.end());
    AdjustSeqIndexes();
    AdjustShuffleRange();
    m_MutexForQue.Unlock();
}

void Playlist::RemoveItem(int index)
{
    m_MutexForQue.Lock();
    m_ItemQue.erase(m_ItemQue.begin() + index);
    AdjustSeqIndexes();
    AdjustShuffleRange();
    m_MutexForQue.Unlock();
}

void Playlist::RemoveItem(const vector<int>& indexes)
{
    m_MutexForQue.Lock();
    for (int i = indexes.size()-1; i >= 0; --i) {
        m_ItemQue.erase(m_ItemQue.begin() + indexes[i]);
    }
    AdjustSeqIndexes();
    AdjustShuffleRange();
    m_MutexForQue.Unlock();
}

void Playlist::Clear()
{
    m_MutexForQue.Lock();
    m_ItemQue.clear();
    m_SeqShuffleQue.clear();
    AdjustSeqIndexes();
    m_MutexForQue.Unlock();
}

void* Playlist::GetItem(int index)
{
    m_MutexForQue.Lock();
    void* item = m_ItemQue[index];
    m_MutexForQue.Unlock();
    return item;
}

int Playlist::GetItemCount() const
{
    m_MutexForQue.Lock();
    int size = m_ItemQue.size();
    m_MutexForQue.Unlock();
    return size;
}

bool Playlist::Empty() const
{
    m_MutexForQue.Lock();
    bool empty = m_ItemQue.empty();
    m_MutexForQue.Unlock();
    return empty;
}

void Playlist::Reverse()
{
    m_MutexForQue.Lock();
    reverse(m_ItemQue.begin(), m_ItemQue.end());
    m_MutexForQue.Unlock();
}

void Playlist::AdjustSeqIndexes()
{
    if (!m_ItemQue.empty()) {
        if (m_SeqNormalIndex == -1)
            m_SeqNormalIndex = 0;
        if (m_SeqRepeatIndex == -1)
            m_SeqRepeatIndex = 0;
        if (m_SeqShuffleIndex == -1)
            m_SeqShuffleIndex = 0;
    } else {
        m_SeqNormalIndex = -1;
        m_SeqRepeatIndex = -1;
        m_SeqShuffleIndex = -1;
    }
}

void Playlist::AdjustShuffleRange(bool reGenerate)
{
    if (reGenerate)
        m_SeqShuffleQue.clear();

    int need = m_ItemQue.size() - m_SeqShuffleQue.size();
    if (need > 0) {
        srandomdev();
        for (int i = 0; i < need; ++i) {
            int inspos = random() % (m_SeqShuffleQue.size()+1);
            m_SeqShuffleQue.insert(m_SeqShuffleQue.begin()+inspos, m_SeqShuffleQue.size());
        }
    } else if (need < 0){
        int top = m_ItemQue.size();
        for (int i = m_SeqShuffleQue.size()-1; i >= 0; --i) {
            if (m_SeqShuffleQue[i] >= top)
                m_SeqShuffleQue.erase(m_SeqShuffleQue.begin() + i);
        }
    }

    assert(m_ItemQue.size() == m_SeqShuffleQue.size());

    // debug
    cout << "shuffle:";
    for (size_t i = 0; i < m_SeqShuffleQue.size(); ++i) {
        cout << m_SeqShuffleQue[i] << ", ";
    }
    cout << endl;
}
