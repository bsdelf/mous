#include "Playlist.h"
#include <cstdlib>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <scx/Mutex.hpp>
using namespace std;
using namespace scx;
using namespace mous;

Playlist::Playlist():
    m_PlayMode(PlayMode::Normal),
    m_MutexForQue(new Mutex),
    m_SeqNormalIndex(-1),
    m_SeqShuffleIndex(-1)
{

}

Playlist::~Playlist()
{
    delete m_MutexForQue;
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
    m_MutexForQue->Lock();
    CorrectSeqIndexes();
    switch (m_PlayMode) {
        case PlayMode::Normal:
        case PlayMode::Repeat:
        case PlayMode::RepeatOne:
        {
            int index = CorrectIndex(m_SeqNormalIndex + off);
            if (index >= 0)
                item = m_ItemQue[index];
        }
            break;

        case PlayMode::Shuffle:
        case PlayMode::ShuffleRepeat:
        {
            int index = CorrectIndex(m_SeqShuffleIndex + off);
            if (index >= 0)
                item = m_ItemQue[index];
        }
            break;
    }
    m_MutexForQue->Unlock();
    return item;
}

EmErrorCode Playlist::SeqJumpTo(int index) const
{
    EmErrorCode ret = ErrorCode::Ok;
    m_MutexForQue->Lock();
    CorrectSeqIndexes();
    switch (m_PlayMode) {
        case PlayMode::Normal:
            break;

        case PlayMode::Repeat:
            break;

        case PlayMode::Shuffle:
            break;

        case PlayMode::ShuffleRepeat:
            break;

        case PlayMode::RepeatOne:
            break;
    }
    m_MutexForQue->Unlock();
    return ret;
}

EmErrorCode Playlist::SeqMoveNext(int step) const
{
    EmErrorCode ret = ErrorCode::Ok;
    m_MutexForQue->Lock();
    CorrectSeqIndexes();
    switch (m_PlayMode) {
        case PlayMode::Normal:
            break;

        case PlayMode::Repeat:
            break;

        case PlayMode::Shuffle:
            break;

        case PlayMode::ShuffleRepeat:
            break;

        case PlayMode::RepeatOne:
            break;
    }
    m_MutexForQue->Unlock();
    return ret;
}

void Playlist::AssignItems(std::deque<void*>& items)
{
    m_MutexForQue->Lock();
    m_ItemQue.assign(items.begin(), items.end());
    AdjustShuffleRange(true);
    m_MutexForQue->Unlock();
}

void Playlist::InsertItem(int index, void* item)
{
    m_MutexForQue->Lock();
    m_ItemQue.insert(m_ItemQue.begin()+index, item);
    AdjustShuffleRange();
    m_MutexForQue->Unlock();
}

void Playlist::InsertItem(int index, deque<void*>& items)
{
    m_MutexForQue->Lock();
    m_ItemQue.insert(m_ItemQue.begin()+index, items.begin(), items.end());
    AdjustShuffleRange();
    m_MutexForQue->Unlock();
}

void Playlist::AppendItem(void* item)
{
    m_MutexForQue->Lock();
    m_ItemQue.push_back(item);
    AdjustShuffleRange();
    m_MutexForQue->Unlock();
}

void Playlist::AppendItem(deque<void*>& items)
{
    m_MutexForQue->Lock();
    m_ItemQue.insert(m_ItemQue.end(), items.begin(), items.end());
    AdjustShuffleRange();
    m_MutexForQue->Unlock();
}

void Playlist::RemoveItem(int index)
{
    m_MutexForQue->Lock();
    m_ItemQue.erase(m_ItemQue.begin() + index);
    AdjustShuffleRange();
    m_MutexForQue->Unlock();
}

void Playlist::RemoveItem(const vector<int>& indexes)
{
    m_MutexForQue->Lock();
    for (int i = indexes.size()-1; i >= 0; --i) {
        m_ItemQue.erase(m_ItemQue.begin() + indexes[i]);
    }
    AdjustShuffleRange();
    m_MutexForQue->Unlock();
}

void Playlist::Clear()
{
    m_MutexForQue->Lock();
    m_ItemQue.clear();
    m_SeqShuffleQue.clear();
    m_MutexForQue->Unlock();
}

void* Playlist::GetItem(int index)
{
    m_MutexForQue->Lock();
    void* item = m_ItemQue[index];
    m_MutexForQue->Unlock();
    return item;
}

int Playlist::GetItemCount() const
{
    m_MutexForQue->Lock();
    int size = m_ItemQue.size();
    m_MutexForQue->Unlock();
    return size;
}

bool Playlist::Empty() const
{
    m_MutexForQue->Lock();
    bool empty = m_ItemQue.empty();
    m_MutexForQue->Unlock();
    return empty;
}

void Playlist::Reverse()
{
    m_MutexForQue->Lock();
    reverse(m_ItemQue.begin(), m_ItemQue.end());
    m_MutexForQue->Unlock();
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

void Playlist::CorrectSeqIndexes() const
{
    m_SeqNormalIndex = CorrectIndex(m_SeqNormalIndex);
    m_SeqShuffleIndex = CorrectIndex(m_SeqShuffleIndex);
}

int Playlist::CorrectIndex(int index) const
{
    if (index >= (int)m_ItemQue.size())
        index = m_ItemQue.size()-1;
    else if (index < 0)
        index = -1;
    return index;
}

