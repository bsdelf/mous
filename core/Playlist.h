#ifndef MOUS_PLAYLIST_H
#define MOUS_PLAYLIST_H

#include <core/IPlaylist.h>
#include <scx/Mutex.hpp>
#include <common/MediaItem.h>

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

    void AssignItem(std::deque<MediaItem*>& items);
    void InsertItem(int index, MediaItem* item);
    void InsertItem(int index, std::deque<MediaItem*>& items);
    void AppendItem(MediaItem* item);
    void AppendItem(std::deque<MediaItem*>& items);
    void RemoveItem(int index);
    void RemoveItem(const std::vector<int>& indexes);
    void Clear();

    MediaItem* GetItem(int index);
    int GetItemCount() const;
    bool Empty() const;
    void Reverse();

private:
    void AdjustSeqIndexes();
    void AdjustShuffleRange(bool reGenerate = false);

private:
    EmPlayMode m_PlayMode;
    mutable scx::Mutex m_MutexForQue;

    std::deque<MediaItem*> m_ItemQue;
    typedef std::deque<MediaItem*>::iterator ItemQueIter;

    mutable int m_SeqNormalIndex;
    mutable int m_SeqRepeatIndex;
    mutable int m_SeqShuffleIndex;
    std::deque<int> m_SeqShuffleQue;
};

}

#endif
