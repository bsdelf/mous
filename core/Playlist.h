#ifndef MOUS_PLAYLIST_H
#define MOUS_PLAYLIST_H

#include <vector>
#include <deque>
#include <mous/ErrorCode.h>
#include <mous/MediaItem.h>

namespace scx {
    class Mutex;
}

namespace mous {

namespace PlayMode {
enum e
{
    Normal,
    Repeat,
    Shuffle,
    ShuffleRepeat,
    RepeatOne
};
}
typedef PlayMode::e EmPlayMode;

class Playlist
{
public:
    Playlist();
    ~Playlist();

    void SetPlayMode(EmPlayMode mode);
    EmPlayMode GetPlayMode() const;

    const MediaItem* SeqCurrent(int off = 0);
    EmErrorCode SeqJumpTo(int index);
    EmErrorCode SeqMoveNext(int step = 1); 

    void AssignItems(std::deque<MediaItem*>& items);
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
    void AdjustShuffleRange(bool reGenerate = false);
    void CorrectSeqIndexes();
    inline int CorrectIndex(int index) const;

private:
    EmPlayMode m_PlayMode;
    scx::Mutex* m_MutexForQue;

    std::deque<MediaItem*> m_ItemQue;
    typedef std::deque<MediaItem*>::iterator ItemQueIter;

    int m_SeqNormalIndex;
    int m_SeqShuffleIndex;
    std::deque<int> m_SeqShuffleQue;
};

}

#endif
