#ifndef MOUS_PLAYLIST_H
#define MOUS_PLAYLIST_H

#include <core/IPlaylist.h>
#include <scx/Mutex.hpp>

namespace mous {

class Playlist: public IPlaylist
{
public:
    Playlist();
    ~Playlist();

    void SetPlayMode(EmPlayMode mode);
    EmPlayMode GetPlayMode() const;

    const void* SeqCurrent(int off = 0) const;
    EmErrorCode SeqJumpTo(int index) const;
    EmErrorCode SeqMoveNext(int step = 1) const; 

    void AssignItems(std::deque<void*>& items);
    void InsertItem(int index, void* item);
    void InsertItem(int index, std::deque<void*>& items);
    void AppendItem(void* item);
    void AppendItem(std::deque<void*>& items);
    void RemoveItem(int index);
    void RemoveItem(const std::vector<int>& indexes);
    void Clear();

    void* GetItem(int index);
    int GetItemCount() const;
    bool Empty() const;
    void Reverse();

private:
    void AdjustSeqIndexes();
    void AdjustShuffleRange(bool reGenerate = false);

private:
    EmPlayMode m_PlayMode;
    mutable scx::Mutex m_MutexForQue;

    std::deque<void*> m_ItemQue;
    typedef std::deque<void*>::iterator ItemQueIter;

    mutable int m_SeqNormalIndex;
    mutable int m_SeqRepeatIndex;
    mutable int m_SeqShuffleIndex;
    std::deque<int> m_SeqShuffleQue;
};

}

#endif
